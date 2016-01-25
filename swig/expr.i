%ignore extfun_t;
%ignore funcset_t;
%ignore extlang_t;
%ignore extlang;
%ignore extlangs_t;
%ignore extlangs;
%ignore register_extlang;
%ignore IDCFuncs;
%ignore set_idc_func;
%ignore set_idc_dtor;
%ignore set_idc_method;
%ignore set_idc_getattr;
%ignore set_idc_setattr;
%ignore set_idc_func_ex;
%ignore run_statements_idc;
%ignore VarLong;
%ignore VarNum;
%ignore extlang_get_attr_exists;
%ignore extlang_create_object_exists;
%ignore create_script_object;
%ignore set_script_attr;
%ignore set_attr_exists;
%ignore get_script_attr;
%ignore extlang_get_attr_exists;
%ignore extlang_compile_file;
%ignore get_extlangs;
%ignore create_idc_object;
%ignore run_script_func;
%ignore VarString;
%ignore VarFloat;
%ignore VarFree;
%ignore calcexpr_long;
%ignore Run;
%ignore ExecuteLine;
%ignore ExecuteFile;
%ignore set_idc_func_body;
%ignore get_idc_func_body;
%ignore idc_stacksize;
%ignore idc_calldepth;
%ignore expr_printf;
%ignore expr_uprintf;
%ignore expr_sprintf;
%ignore expr_printfer;
%ignore init_idc;
%ignore term_idc;
%ignore create_default_idc_classes;
%ignore notify_extlang_changed;
%ignore insn_to_idc;
%ignore find_builtin_idc_func;
%ignore idc_mutex;
%ignore idc_lx;
%ignore idc_vars;
%ignore idc_resolve_label;
%ignore idc_resolver_ea;
%ignore setup_lowcnd_regfuncs;
%ignore syntax_highlighter_t;
%cstring_output_maxstr_none(char *errbuf, size_t errbufsize);

%ignore CompileEx;
%rename (CompileEx) CompileEx_wrap;
%ignore Compile;
%rename (Compile) Compile_wrap;
%ignore calcexpr;
%rename (calcexpr) calcexpr_wrap;
%ignore calc_idc_expr;
%rename (calc_idc_expr) calc_idc_expr_wrap;
%ignore CompileLine(const char *line, char *errbuf, size_t errbufsize, uval_t (idaapi*_getname)(const char *name)=NULL);
%ignore CompileLineEx;
%ignore CompileLine;
%rename (CompileLine) CompileLine_wrap;
%{
//<code(py_expr)>
//</code(py_expr)>
%}

%inline %{
//<inline(py_expr)>
//</inline(py_expr)>
%}

%include "expr.hpp"

%pythoncode %{
#<pycode(py_expr)>
#</pycode(py_expr)>
%}
