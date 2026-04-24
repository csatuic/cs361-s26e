#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "utils.h"
#include "config.h"

int filter_entries(LogEntry *entries, int count, int min_severity, time_t start_time, time_t end_time);
int compute_severity_score(int severity);
void build_event_correlations(LogEntry *entries, int count, int correlation_matrix[][5]);
int aggregate_hourly_stats(LogEntry *entries, int count, int hourly_counts[24]);
void filter_by_event_type(LogEntry *entries, int *count, const char *event_filter);
void remove_duplicates(LogEntry *entries, int *count);

#endif