/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
class EHI {
public:
	int eh_interactive(void);
	virtual int execute_cmd(char *rawcmd, ehvar_t **paras);
	virtual ~EHI();
};
extern EHI *interpreter;
