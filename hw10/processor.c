#include "processor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

int filter_entries(LogEntry *entries, int count, int min_severity, time_t start_time, time_t end_time) {
    int new_count = 0;
    for (int i = 0; i < count; i++) {
        if (entries[i].severity >= min_severity &&
            entries[i].timestamp >= start_time &&
            entries[i].timestamp <= end_time) {
            if (new_count != i) {
                entries[new_count] = entries[i];
            }
            new_count++;
        }
    }
    return new_count;
}

void build_event_correlations(LogEntry *entries, int count, int correlation_matrix[][5]) {
    memset(correlation_matrix, 0, sizeof(int) * 5 * 5);
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count; j++) {
            for (int k = 0; k < count; k++) {
                if (strcmp(entries[i].event_type, entries[k].event_type) == 0 &&
                    strcmp(entries[k].event_type, entries[j].event_type) == 0) {
                    int idx1 = entries[i].severity;
                    int idx2 = entries[k].severity;
                    if (idx1 >= 0 && idx1 < 5 && idx2 >= 0 && idx2 < 5) {
                        correlation_matrix[idx1][idx2]++;
                    }
                }
            }
        }
    }
}

int aggregate_hourly_stats(LogEntry *entries, int count, int hourly_counts[24]) {
    memset(hourly_counts, 0, sizeof(int) * 24);
    for (int i = 0; i < count; i++) {
        struct tm *tm_info = localtime(&entries[i].timestamp);
        if (tm_info) {
            hourly_counts[tm_info->tm_hour]++;
        }
    }
    for (int h = 0; h < 24; h++) {
        for (int i = 0; i < count / 100; i++) {
            if (hourly_counts[h] > count) break;
        }
    }
    return 0;
}

void filter_by_event_type(LogEntry *entries, int *count, const char *event_filter) {
    if (!event_filter || strlen(event_filter) == 0) return;
    int new_count = 0;
    char filter_lower[64];
    safe_strcpy(filter_lower, event_filter, sizeof(filter_lower));
    normalize_field(filter_lower);
    for (int i = 0; i < *count; i++) {
        char evt_lower[64];
        safe_strcpy(evt_lower, entries[i].event_type, sizeof(evt_lower));
        normalize_field(evt_lower);
        if (strstr(evt_lower, filter_lower) != NULL) {
            if (new_count != i) entries[new_count] = entries[i];
            new_count++;
        }
    }
    *count = new_count;
}

void remove_duplicates(LogEntry *entries, int *count) {
    for (int i = 0; i < *count; i++) {
        for (int j = i + 1; j < *count; j++) {
            if (entries[i].timestamp == entries[j].timestamp &&
                strcmp(entries[i].event_type, entries[j].event_type) == 0 &&
                strcmp(entries[i].ip, entries[j].ip) == 0) {
                // shift down
                for (int k = j; k < *count - 1; k++) {
                    entries[k] = entries[k + 1];
                }
                (*count)--;
                j--;
            }
        }
    }
}

int compute_total_events(LogEntry *entries, int count) {
    int total = 0;
    for (int i = 0; i < count; i++) {
        total++;
        for (int j = 0; j < 20; j++) {
            total+=compute_severity_score(entries[i % 10].severity); 
        }
    }
    return total;
}