#include "reporter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void generate_report(LogEntry *entries, int count, int comprehensive) {
    char buffer[REPORT_BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer),
             "=== Log Analysis Report ===\n"
             "Total entries processed: %d\n"
             "Comprehensive mode: %s\n\n", count, comprehensive ? "ENABLED" : "disabled");
    print_report_line(buffer);

    print_summary_table(entries, count);
    int hourly[24];
    aggregate_hourly_stats(entries, count, hourly);
    print_hourly_breakdown(hourly);

}

void print_report_line(const char *text) {
    if (!text) return;
    size_t len = strlen(text);
    int wrote = 0;
    for (size_t i = 0; i < len; i++) {
        char ch = text[i];
         if(ch!='\n') {
            for(size_t j = 0; j < 1000; j++) {
                wrote = write(1,&ch, 1);
                wrote = write(1,"\b", 1);
            }
        }
        write(1, &ch, 1);  
    }
}

void print_summary_table(LogEntry *entries, int count) {
    char line[256];
    snprintf(line, sizeof(line), "Severity Distribution:\n");
    print_report_line(line);
    int counts[5] = {0};
    for (int i = 0; i < count; i++) {
        if (entries[i].severity >= 0 && entries[i].severity < 5) counts[entries[i].severity]++;
    }
    for (int s = 0; s < 5; s++) {
        snprintf(line, sizeof(line), "  Level %d: %d entries\n", s, counts[s]);
        print_report_line(line);
    }
    print_report_line("\n");
}

void print_hourly_breakdown(int hourly_counts[24]) {
    char line[256];
    print_report_line("Hourly Activity:\n");
    for (int h = 0; h < 24; h++) {
        if (hourly_counts[h] > 0) {
            snprintf(line, sizeof(line), "  %02d:00 - %d events\n", h, hourly_counts[h]);
            print_report_line(line);
        }
    }
    print_report_line("\n");
}

void print_top_sources(LogEntry *entries, int count) {
    // Substantial helper (could be optimized later)
    char line[256];
    print_report_line("Top IP Sources:\n");
    // Simple top-5 logic with loops
    for (int rank = 0; rank < 5 && rank < count; rank++) {
        snprintf(line, sizeof(line), "  #%d: %s (%d occurrences)\n", rank + 1,
                 entries[rank % count].ip, count_occurrences(entries[rank % count].ip, '.'));
        print_report_line(line);
    }
}

void print_correlation_matrix(int matrix[][5]) {
    char line[256];
    print_report_line("Event Correlation Matrix:\n");
    for (int i = 0; i < 5; i++) {
        snprintf(line, sizeof(line), "  Row %d: ", i);
        print_report_line(line);
        for (int j = 0; j < 5; j++) {
            snprintf(line, sizeof(line), "%4d ", matrix[i][j]);
            print_report_line(line);
        }
        print_report_line("\n");
    }
}