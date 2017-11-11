
/*
 * Implementation of tables of structs in .rodata. This can be used for
 * registering all kind of objects throughout the code at compile time, for
 * example event or cmd handlers.
 *
 * A romtab consists of a number of structs which are initialized at compile time
 * and are all placed in a specific section by the linker. The ROMTAB() macro
 * can be used to add an entry to a romtab, the ROMTAB_FOREACH() macro is used
 * for iterating through all entries in a romtab, and calling the given callback
 * function for each entry.
 */

#ifndef bios_romtab_h
#define bios_romtab_h

/*
 * Macro for defining an entry in a romtab
 */

#define PASTE1(x, y) x ## y
#define PASTE2(x, y) PASTE1(x, y)
#define UNIQ_ID(n) PASTE2(n, __LINE__)

#define ROMTAB(type) \
	static const struct type \
	__attribute__((section(#type), used, aligned(1))) \
	UNIQ_ID(type) =


/*
 * Macro for iterating a romtab. Returns the last encountered rv code
 * that is not RV_OK
 */

#define ROMTAB_FOREACH(tab, fn, fndata) ({    \
	extern struct tab __start_ ## tab;    \
	extern struct tab __stop_ ## tab;     \
	struct tab *l = &__start_ ## tab;     \
	int r = 0;                            \
	while(l < &__stop_ ## tab) {          \
		int r2 = fn(l++, fndata);     \
		if(r2 != 0) r = r2;           \
	}                                     \
	r;                                    \
	})                                    \


#endif
