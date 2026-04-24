#ifndef REPORTER_H
#define REPORTER_H

#include "utils.h"
#include "config.h"

void generate_report(LogEntry *entries, int count, int comprehensive);
void print_report_line(const char *text);  
void print_summary_table(LogEntry *entries, int count);
void print_hourly_breakdown(int hourly_counts[24]);
void print_top_sources(LogEntry *entries, int count);
void print_correlation_matrix(int matrix[][5]);

#endif