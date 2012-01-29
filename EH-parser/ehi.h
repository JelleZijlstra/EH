/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
class EHI {
public:
	int eh_interactive(void);
	virtual ehretval_t execute_cmd(const char *rawcmd, ehvar_t **paras);
	virtual void exec_file(const char *name);
	virtual char *eh_getline(void);
	virtual ~EHI();
};
extern EHI *interpreter;
