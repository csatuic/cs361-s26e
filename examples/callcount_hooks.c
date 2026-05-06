

// profiling functions below
extern int call_count;
extern void target_func(void);


void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    if (this_fn == (void *)target_func) {
	printf("called target func!\n");
        call_count++;
    }
}

void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    // nothing needed for a simple call counter
}
