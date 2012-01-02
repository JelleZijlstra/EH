CC = g++
CCFLAGS = -ggdb -Wall
SOURCES = lex.yy.c y.tab.c eh_tree.c eh_error.c
COMPILER = $(SOURCES) eh_compiler.c
COMPILER_O = $(COMPILER:.c=.o)
INTERPRETER = $(SOURCES) eh_libfuncs.c eh_hash.c eh_interpreter.c
EHI = $(INTERPRETER) eh_cli.c
EHI_O = $(EHI:.c=.o)
EHPHP = $(INTERPRETER) eh_php.c eh_wrap.cpp
EHPHP_O1 = $(EHPHP:.c=.o)
EHPHP_O = $(EHPHP_O1:.cpp=.o)
SYMBOLS = -Wl,-flat_namespace,-U,_zend_register_resource,-U,_zend_rsrc_list_get_rsrc_type,-U,_zend_wrong_param_count,-U,_compiler_globals,-U,_zend_hash_find,-U,_gc_remove_zval_from_buffer,-U,_zend_register_long_constant,-U,_zend_get_constant,-U,__efree,-U,__emalloc,-U,_zend_error,-U,__zend_list_find,-U,_main,-U,__zend_get_parameters_array_ex,-U,_zend_register_list_destructors_ex,-U,__zval_copy_ctor_func,-U,__convert_to_string,-U,__zend_hash_add_or_update,-U,_executor_globals,-U,_zval_used_for_init,-U,_zval_is_true,-U,__object_init,-U,_php_strtolower,-U,__estrndup,-U,__object_init_ex,-U,_zend_lookup_class,-U,_call_user_function,-U,_convert_to_long,-U,_add_index_long,-U,_add_assoc_string_ex,-U,_add_assoc_long_ex,-U,_add_index_bool,-U,__array_init,-U,_add_assoc_bool_ex,-U,_add_index_string,-U,_call_user_function_ex

eh_wrap.o: eh_wrap.cpp
	$(CC) `php-config --includes` -fPIC -c $< -o $@

%.o: %.c
	$(CC) -fPIC -c $(CCFLAGS) $< -o $@

compiler: $(COMPILER_O)
	$(CC) $(COMPILER_O) -o ehc

ehi: $(EHI_O)
	$(CC) $(EHI_O) -o ehi

ehphp: $(EHPHP_O)
	$(CC) -dynamiclib $(SYMBOLS) -o ehphp.dylib $(EHPHP_O)
	sudo cp ehphp.dylib /opt/local/lib/php/extensions/no-debug-non-zts-20090626/ehphp.dylib

y.tab.c y.tab.h: eh.y
	yacc -d eh.y

lex.yy.c: eh.l y.tab.h
	lex eh.l

eh_wrap.cpp: eh.i
	swig -c++ -php5 eh.i
	# Need to change the generated .php files
	sed -e "s/'\.PHP_SHLIB_SUFFIX/dylib'/" -i '.save' ehphp.php 

clean:
	rm -f *.o ehphp.so ehphp.dylib ehi ehc y.tab.h y.tab.c lex.yy.c y.output eh_wrap.c ehphp.php ehphp.php.save eh_wrap.cpp php_ehphp.h eh_wrap.h
