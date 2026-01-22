use hypher::{hyphenate, Lang};
use rand::seq::SliceRandom;
use std::fs::File;
use std::io::{self, BufRead, BufReader, Write};
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
    let _ = write!(write_to, "\n");
}

fn main() -> io::Result<()> {
    let opt = Opt::from_args();
    let filename = opt.file.unwrap_or_else(|| PathBuf::from("limericks.txt"));
    assert!(filename.as_path().exists(),"Limerick file {filename:?} not found. Specify correct file path with -f. ");
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

    let mut outfile;
    let writer : &mut dyn Write = 
    if let Some(name) = &opt.outfile {
        sleep(Duration::from_millis(1000));
        outfile = File::create(name).unwrap();
        &mut outfile
    }
    else {
        &mut io::stdout()
    };

    let mut rng = rand::thread_rng();
    if !opt.live {
        let chosen = limericks.choose(&mut rng).unwrap();
        let _ = write!(writer, "{}\n", chosen);
        return Ok(());
    }

    loop {
        let chosen = limericks.choose(&mut rng).unwrap();
        present_limerick(writer, &stop, chosen);
    }
}
