#if (!defined(colljp_h))
 
#define colljp_h
 
#if (defined(HAS_FUNCTION_PROTOTYPE))
    int     colljp(const UCHAR *left, const UCHAR *right,
                int left_len, int right_len, const UNIV *table);
 
            /*  do collation in Japanese locale.
             *  collate strings from top of `left'  until `left_len'  bytes,
             *          with    from top of `right' until `right_len' bytes,
             *  left,right          : strings to compare.
             *  left_len, right_len : bytes of strings (from top of string)
             *  table               : return of opncoll()
             */
 
    UNIV    *opncoll(void);
 
            /*  initialize colljp routines.
             *  returns table for colljp().
             */
 
    int     clscoll(UNIV *table);
 
            /*  frees memory for colljp routines.
             *  table : return of opncoll()
             */
#else
    int     colljp();
    UNIV    *opncoll();
    int     clscoll();
#endif
 
#endif
