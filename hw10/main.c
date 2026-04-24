#include "log_reader.h"
#include "parser.h"
#include "processor.h"
#include "reporter.h"
#include "utils.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static void print_usage(const char *progname);
void orchestrate_analysis(const char *filename, int comprehensive);

int main(int argc, char **argv) {
    int comprehensive = 0;
    char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--comprehensive") == 0 || strcmp(argv[i], "-c") == 0) {
            comprehensive = 1;
        } else if (filename == NULL) {
            filename = argv[i];
        }
    }

    if (filename == NULL) {
        print_usage(argv[0]);
        return 1;
    }

    orchestrate_analysis(filename, comprehensive);
    return 0;
}

static void print_usage(const char *progname) {
    printf("Usage: %s [--comprehensive | -c] <logfile>\n", progname);
    printf("  --comprehensive : Enable deeper analysis (triggers object-file functions)\n");
}

__attribute__((noinline))
void orchestrate_analysis(const char *filename, int comprehensive) {
    sleep(8);

    LogEntry *entries = malloc(MAX_ENTRIES * sizeof(LogEntry));
    if (!entries) {
        fprintf(stderr, "Error: Out of memory\n");
        return;
    }

    int count = read_log_file(filename, entries, MAX_ENTRIES);
    if (count < 0) {
        free(entries);
        return;
    }

    count = parse_log_entries(filename, entries, MAX_ENTRIES);
    sanitize_entries(entries, count);
    count = filter_entries(entries, count, 0, 0, time(NULL) + 86400);


    if (comprehensive) {
        for (int i = 0; i < count; i += 50) {        
            count+=compute_severity_score(entries[i].severity);
        }
        process_log_batch(entries, count);              
        compute_detailed_statistics(entries, count, 1);  
    }

    int matrix[5][5];
    build_event_correlations(entries, count, matrix);
    generate_report(entries, count, comprehensive);
    print_correlation_matrix(matrix);

    free(entries);
}
