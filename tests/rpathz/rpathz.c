extern int rpathx_value ();
extern int rpathy_value ();
int rpathz_value () { return 1000 * rpathx_value () + 3 * rpathy_value (); }
