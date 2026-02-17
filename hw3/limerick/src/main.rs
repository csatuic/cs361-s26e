use hypher::{hyphenate, Lang};
use rand::seq::SliceRandom;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Write, Read, Lines};
use std::path::PathBuf;
use std::thread::sleep;
use std::time::Duration;
use structopt::StructOpt;
use std::sync::*;
use ctrlc;

#[derive(StructOpt, Debug)]
#[structopt(name = "limerick-presenter", about = "Presents random limericks, optionally syllable-by-syllable in live mode.")]
struct Opt {
    /// Enable live presentation mode (syllable-by-syllable with pauses)
    #[structopt(short = "l", long = "live")]
    live: bool,

    /// Input file containing limericks (default: limericks.txt)
    #[structopt(short = "f", long = "file", parse(from_os_str))]
    file: Option<PathBuf>,

    /// Output file containing limericks (default: limericks.txt)
    #[structopt(short = "o", long = "outfile", parse(from_os_str))]
    outfile: Option<PathBuf>,

    #[structopt(short = "s", long = "server")]
    server: Option<String>,
}

fn load_limericks(path: PathBuf) -> io::Result<Vec<String>> {
    let file = File::open(path)?;
    let reader = BufReader::new(file);
    let mut limericks = Vec::new();
    let mut current = String::new();

    for line in reader.lines() {
        let line = line?;
        if line.trim().is_empty() {
            if !current.trim().is_empty() {
                limericks.push(current.trim_end().to_string());
                current.clear();
            }
        } else {
            current.push_str(&line);
            current.push('\n');
        }
    }

    if !current.trim().is_empty() {
        limericks.push(current.trim_end().to_string());
    }

    Ok(limericks)
}

fn present_limerick(write_to: &mut dyn Write, stop: &Arc<Mutex<bool>>, limerick: &str) {
    for line in limerick.lines() {
        let mut words = line.split_whitespace().peekable();

        while let Some(word) = words.next() {
            let cleaned = word.trim_matches(|c: char| !c.is_alphabetic() && c != '\'');
            if !cleaned.is_empty() {
                for syllable in hyphenate(cleaned, Lang::English).collect::<Vec<_>>() {
                    if *stop.lock().unwrap() == true {
                        *stop.lock().unwrap() = false;
                        return ;
                    }

                    let _ = write!(write_to,"{}",syllable);
                    write_to.flush().unwrap();
                    sleep(Duration::from_millis(200));                                
                }

                let _ = write!(write_to, " ");
                write_to.flush().unwrap();
            }
        }

        let _ = write!(write_to, "\n");
        write_to.flush().unwrap();
        sleep(Duration::from_millis(800));
    }
}

fn await_limerick(from: &mut dyn BufRead) -> io::Result<String> {
    let mut lines = from.lines();

    let one=&lines.next().unwrap()?;
    let two=&lines.next().unwrap()?;
    let three=&lines.next().unwrap()?;
    let four=&lines.next().unwrap()?;
    let five=&lines.next().unwrap()?;

    println!("Poem received. Checking for Limerick");

    // Turns out this was far too complex for a systems course. 
    // Maybe ask the machine learning folks!
    std::thread::sleep(std::time::Duration::from_secs(1));
    println!("Uh... I dunno. Maybe?");

    Ok(format!("{one}\n{two}\n{three}\n{four}\n{five}"))
}

fn expect_exact(from: &mut dyn Read, exact_string: &str) -> Result<(),String> {
    let mut buf = vec![0u8;exact_string.as_bytes().len()];
    from.read_exact(&mut buf);
    if buf == exact_string.as_bytes() {
        Ok(())
    }
    else { 
        Err(format!("Did not get '{}'",exact_string))
    }
}

fn main() -> io::Result<()> {
    let opt = Opt::from_args();
    let filename = opt.file.unwrap_or_else(|| PathBuf::from("limericks.txt"));
    if !filename.as_path().exists() {
        println!("Limerick file {filename:?} not found. Specify correct file path with -f. ");
        return Ok(());
    }
    let limericks = load_limericks(filename)?;

    if limericks.is_empty() {
        eprintln!("No limericks found in the file.");
        return Ok(());
    }

    let stop = Arc::new(Mutex::new(false));
    let alsostop = stop.clone();
    ctrlc::set_handler(move || {
        println!("Eh-hrm...");
        *alsostop.lock().unwrap()=true;
    }).expect("Error setting Ctrl-C handler");
    let mut rng = rand::thread_rng();

    if let Some(name) = &opt.outfile {
        let mut outfile;
        sleep(Duration::from_millis(1000));
        outfile = File::create(name).unwrap();

        let chosen = limericks.choose(&mut rng).unwrap();
        let _ = write!(outfile, "{}\n", chosen);
        return Ok(());
    }
    else if let Some(server) = &opt.server {
        let mut socket = std::net::TcpStream::connect(server).unwrap();
        let mut bufreader = BufReader::new(socket.try_clone()?);

        if !opt.live {
            let chosen = limericks.choose(&mut rng).unwrap();
            let _ = write!(socket, "PRESENT\n");
            expect_exact(&mut bufreader,"GO AHEAD\n").unwrap();
            let _ = write!(socket, "{}\n", chosen);            
            let _ = write!(socket, "AWAIT\n");
            await_limerick(&mut bufreader);
        }
        else {
            loop {
                let chosen = limericks.choose(&mut rng).unwrap();
                let _ = write!(socket, "PRESENT\n");
                expect_exact(&mut bufreader,"GO AHEAD\n").unwrap();
                let _ = present_limerick(&mut socket, &stop, chosen);            
                let _ = write!(socket,"AWAIT\n");
                await_limerick(&mut bufreader);            
            }
        }
    }
    else {
        if !opt.live {
            let chosen = limericks.choose(&mut rng).unwrap();
            let _ = write!(&mut io::stdout(), "{}\n", chosen);
            return Ok(());
        }
        loop {
            let chosen = limericks.choose(&mut rng).unwrap();
            present_limerick(&mut io::stdout(), &stop, chosen);
        }
    };
    return Ok(())
}
