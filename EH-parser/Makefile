CC = g++
CCBASICFLAGS = -ggdb -fPIC -c
CCFLAGS = $(CCBASICFLAGS) -Wall -pedantic
SOURCES = eh.flex.cpp eh.bison.cpp eh_tree.cpp eh_error.cpp eh_gc.cpp ehi.cpp
COMPILER = $(SOURCES) eh_compiler.cpp
COMPILER_O = $(COMPILER:.cpp=.o)
INTERPRETER = $(SOURCES) eh_libfuncs.cpp eh_libclasses.cpp eh_libcmds.cpp eh_hash.cpp eh_interpreter.cpp
EHI = $(INTERPRETER) eh_cli.cpp
EHI_O = $(EHI:.cpp=.o)
EHPHP = $(INTERPRETER) eh_wrap.cpp
EHPHP_O = $(EHPHP:.cpp=.o)
SYMBOLS = -Wl,-flat_namespace,-U,_zend_register_resource,-U,_zend_rsrc_list_get_rsrc_type,-U,_zend_wrong_param_count,-U,_compiler_globals,-U,_zend_hash_find,-U,_gc_remove_zval_from_buffer,-U,_zend_register_long_constant,-U,_zend_get_constant,-U,__efree,-U,__emalloc,-U,_zend_error,-U,__zend_list_find,-U,_main,-U,__zend_get_parameters_array_ex,-U,_zend_register_list_destructors_ex,-U,__zval_copy_ctor_func,-U,__convert_to_string,-U,__zend_hash_add_or_update,-U,_executor_globals,-U,_zval_used_for_init,-U,_zval_is_true,-U,__object_init,-U,_php_strtolower,-U,__estrndup,-U,__object_init_ex,-U,_zend_lookup_class,-U,_call_user_function,-U,_convert_to_long,-U,_add_index_long,-U,_add_assoc_string_ex,-U,_add_assoc_long_ex,-U,_add_index_bool,-U,__array_init,-U,_add_assoc_bool_ex,-U,_add_index_string,-U,_call_user_function_ex,-U,_add_assoc_zval_ex,-U,_add_index_zval,-U,_add_assoc_double_ex,-U,_add_index_double,-U,_add_assoc_null_ex,-U,_add_index_null

all: ehc ehphp ehi

eh_wrap.o: eh_wrap.cpp
	$(CC) `php-config --includes` $(CCBASICFLAGS) $< -o $@

eh.h: ehi.h eh.bison.hpp

eh_interpreter.cpp: eh_libfuncs.h

eh_libfuncs.cpp: eh_libfuncs.h

%.o: %.cpp eh.h
	$(CC) $(CCFLAGS) $< -o $@

ehc: $(COMPILER_O)
	$(CC) $(COMPILER_O) -o ehc

ehi: $(EHI_O)
	$(CC) $(EHI_O) -o ehi
	sudo cp ehi /usr/bin/ehi

ehphp: $(EHPHP_O)
	./make_ehphp.sh "$(CC)" "$(SYMBOLS)" "$(EHPHP_O)"

# If I don't do "/usr/bin/bison", y.tab.c won't compile because of a const char*
# -> char* conversion.
eh.bison.cpp eh.bison.hpp: eh.y
	/usr/bin/bison -y -d eh.y --output='eh.bison.cpp'

eh.flex.cpp: eh.l eh.bison.hpp
	/opt/local/bin/flex eh.l

eh_wrap.cpp: eh.i
	swig -c++ -php5 eh.i
# Need to change the generated .php files if we're in Mac OS X
	if [ `uname` = 'Darwin' ]; then sed -e "s/'\.PHP_SHLIB_SUFFIX/dylib'/" -e "s/function __construct/protected function __construct/" -i '.save' ehphp.php; fi
# And need to this to get dl to succeed on Linux
	if [ `uname` = 'Linux' ]; then sed -e "s/zend_error_noreturn/zend_error/" -i'.save' eh_wrap.cpp; sed -e "s/function __construct/protected function __construct/" -i'.save' ehphp.php; fi

clean:
	rm -f *.o ehphp.so ehphp.dylib ehi ehc eh.flex.cpp eh.bison.cpp eh.bison.hpp y.output eh_wrap.c ehphp.php ehphp.php.save eh_wrap.cpp php_ehphp.h eh_wrap.h
