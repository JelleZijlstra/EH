# Portability
UNAME := $(shell uname)

CXX = g++
CCBASICFLAGS = -ggdb -fPIC -c -std=c++11
LDFLAGS = -std=c++11

ifeq ($(UNAME), Darwin)
CXX = clang++
CCBASICFLAGS := $(CCBASICFLAGS) -stdlib=libc++
LDFLAGS := $(LDFLAGS) -stdlib=libc++
endif

CCFLAGS = $(CCBASICFLAGS) -Wall -pedantic -Wextra -Wno-c++98-compat -Wno-unused-parameter -Wno-weak-vtables -Wno-padded -Wno-unused-macros -Wno-float-equal
JSON = std_lib/json/
LIBCLASSES = ArgumentError Array Attribute Binding Bool ByteArray Class ConstError EH Enum Exception File Float Function GarbageCollector Generator GlobalObject Hash Integer JSON LoopError MiscellaneousError NameError Node Null Object Range String SyntaxError Tuple TypeError UnknownCommandError EmptyIterator FixedArray Random Map VisibilityError
SOURCES = eh.flex.cpp eh.bison.cpp ehi.cpp eh_types.cpp eh_files.cpp eh_optimize.cpp eh_vm.cpp $(JSON)eh_json.flex.cpp $(JSON)eh_json.bison.cpp
INTERPRETER = $(SOURCES) eh_libcmds.cpp eh_interpreter.cpp $(patsubst %, std_lib/%.cpp, $(LIBCLASSES))
LIBEH = $(INTERPRETER) eh_standalone.cpp
LIBEH_O = $(LIBEH:.cpp=.o)
EHI = $(LIBEH) eh_cli.cpp
EHI_O = $(EHI:.cpp=.o)
EHPHP = $(INTERPRETER) eh_wrap.cpp
EHPHP_O = $(EHPHP:.cpp=.o)
SYMBOLS = -Wl,-flat_namespace,-U,_zend_register_resource,-U,_zend_rsrc_list_get_rsrc_type,-U,_zend_wrong_param_count,-U,_compiler_globals,-U,_zend_hash_find,-U,_gc_remove_zval_from_buffer,-U,_zend_register_long_constant,-U,_zend_get_constant,-U,__efree,-U,__emalloc,-U,_zend_error,-U,__zend_list_find,-U,_main,-U,__zend_get_parameters_array_ex,-U,_zend_register_list_destructors_ex,-U,__zval_copy_ctor_func,-U,__convert_to_string,-U,__zend_hash_add_or_update,-U,_executor_globals,-U,_zval_used_for_init,-U,_zval_is_true,-U,__object_init,-U,_php_strtolower,-U,__estrndup,-U,__object_init_ex,-U,_zend_lookup_class,-U,_call_user_function,-U,_convert_to_long,-U,_add_index_long,-U,_add_assoc_string_ex,-U,_add_assoc_long_ex,-U,_add_index_bool,-U,__array_init,-U,_add_assoc_bool_ex,-U,_add_index_string,-U,_call_user_function_ex,-U,_add_assoc_zval_ex,-U,_add_index_zval,-U,_add_assoc_double_ex,-U,_add_index_double,-U,_add_assoc_null_ex,-U,_add_index_null
EHI_TARGET = ehi

all: ehi libeh ehphp

eh_wrap.o: eh_wrap.cpp
	$(CXX) `php-config55 --includes` $(CCBASICFLAGS) $< -o $@

eh.hpp: ehi.hpp eh.bison.hpp

%.o: %.cpp eh.hpp
	$(CXX) $(CCFLAGS) $< -o $@

ehi: $(EHI_O)
	$(CXX) $(LDFLAGS) $(EHI_O) -o $(EHI_TARGET)
	sudo cp $(EHI_TARGET) /usr/bin/$(EHI_TARGET)

libeh: $(LIBEH_O)
	ar rvs libeh.a $(LIBEH_O) >/dev/null

ehphp: $(EHPHP_O)
	./make_ehphp.sh "$(CXX)" "$(LDFLAGS) $(SYMBOLS)" "$(EHPHP_O)"

eh.bison.cpp eh.bison.hpp: eh.y
	bison -v -y -d eh.y --output='eh.bison.cpp'

eh.flex.cpp: eh.l eh.bison.hpp
	flex --header-file=eh.flex.hpp eh.l

eh_wrap.cpp: eh.i
	swig -c++ -php5 eh.i
	# Need to change the generated .php files if we're in Mac OS X
	if [ `uname` = 'Darwin' ]; then sed -e "s/'\.PHP_SHLIB_SUFFIX/dylib'/" -e "s/function __construct/protected function __construct/" -i '.save' ehphp.php; fi
	# And need to this to get dl to succeed on Linux
	if [ `uname` = 'Linux' ]; then sed -e "s/zend_error_noreturn/zend_error/" -i'.save' eh_wrap.cpp; sed -e "s/function __construct/protected function __construct/" -i'.save' ehphp.php; fi

$(JSON)eh_json.flex.cpp: $(JSON)json.l $(JSON)eh_json.bison.hpp
	cd $(JSON) && flex --header-file=eh_json.flex.hpp json.l

$(JSON)eh_json.bison.cpp $(JSON)eh_json.bison.hpp: $(JSON)json.y
	cd $(JSON) && bison -v -y -d json.y --output='eh_json.bison.cpp'

clean:
	rm -f *.o ehphp.so ehphp.dylib ehi ehc eh.flex.cpp eh.bison.cpp eh.bison.hpp y.output eh_wrap.c ehphp.php ehphp.php.save eh_wrap.cpp php_ehphp.h eh_wrap.h std_lib/*.o
