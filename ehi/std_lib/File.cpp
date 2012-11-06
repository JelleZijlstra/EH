#include "File.h"

EH_INITIALIZER(File) {
	REGISTER_METHOD(File, initialize);
	REGISTER_METHOD(File, open);
	REGISTER_METHOD(File, getc);
	REGISTER_METHOD(File, gets);
	REGISTER_METHOD(File, puts);
	REGISTER_METHOD(File, close);
	REGISTER_METHOD(File, toBool);
	REGISTER_METHOD(File, finalize);
}

EH_METHOD(File, initialize) {
	ehretval_p new_obj = ehretval_t::make_resource(obj->get_full_type(), static_cast<LibraryBaseClass *>(new File()));
	if(args->type() != null_e) {
		ehlm_File_open(new_obj, args, ehi);
	}
	return new_obj;
}
EH_METHOD(File, open) {
	ASSERT_TYPE(args, string_e, "File.open");
	ASSERT_RESOURCE(File, "File.open");
	// close any open file
	if(data->descriptor != NULL) {
		fclose(data->descriptor);
	}

	// and open the new one
	FILE *mfile = fopen(args->get_stringval(), "r+");
	if(mfile == NULL) {
		return ehretval_t::make_bool(false);
	}
	data->descriptor = mfile;
	return ehretval_t::make_bool(true);
}
EH_METHOD(File, getc) {
	ASSERT_TYPE(args, null_e, "File.getc");
	ASSERT_RESOURCE(File, "File.getc");

	if(data->descriptor == NULL) {
		return NULL;
	}
	int c = fgetc(data->descriptor);
	if(c == EOF) {
		return NULL;
	}
	char *out = new char[2];
	out[0] = c;
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(File, gets) {
	ASSERT_TYPE(args, null_e, "File.gets");
	ASSERT_RESOURCE(File, "File.gets");
	if(data->descriptor == NULL) {
		return NULL;
	}
	
	char *out = new char[512];

	char *ptr = fgets(out, 511, data->descriptor);
	if(ptr == NULL) {
		delete[] out;
		return NULL;
	}
	return ehretval_t::make_string(out);
}
EH_METHOD(File, puts) {
	ASSERT_TYPE(args, string_e, "File.puts");
	ASSERT_RESOURCE(File, "File.puts");
	if(data->descriptor == NULL) {
		return NULL;
	}
	
	int count = fputs(args->get_stringval(), data->descriptor);	
	return ehretval_t::make_bool(count != EOF);
}
EH_METHOD(File, close) {
	ASSERT_TYPE(args, null_e, "File.close");
	ASSERT_RESOURCE(File, "File.close");
	if(data->descriptor == NULL) {
		return NULL;
	}
	fclose(data->descriptor);
	data->descriptor = NULL;
	return NULL;
}
EH_METHOD(File, toBool) {
	ASSERT_TYPE(args, null_e, "File.toBool");
	ASSERT_RESOURCE(File, "File.toBool");
	return ehretval_t::make_bool(data->descriptor != NULL);
}
EH_METHOD(File, finalize) {
	ehi->call_method(obj, "close", NULL, obj);
	return NULL;
}
