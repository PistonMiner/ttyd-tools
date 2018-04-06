/* Symbols for us to be able to call global constructors and destructors */
_ctors_start = LOADADDR(.ctors);
_ctors_end = _ctors_start + SIZEOF(.ctors);
_dtors_start = LOADADDR(.dtors);
_dtors_end = _dtors_start + SIZEOF(.dtors);