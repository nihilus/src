
//<code(py_idd)>

//-------------------------------------------------------------------------
static bool dbg_can_query()
{
  // Reject the request only if no debugger is set
  // or the debugger cannot be queried while not in suspended state
  return dbg != NULL && (dbg->may_disturb() || get_process_state() < DSTATE_NOTASK);
}

//-------------------------------------------------------------------------
static PyObject *meminfo_vec_t_to_py(meminfo_vec_t &areas)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  PyObject *py_list = PyList_New(areas.size());
  meminfo_vec_t::const_iterator it, it_end(areas.end());
  Py_ssize_t i = 0;
  for ( it=areas.begin(); it!=it_end; ++it, ++i )
  {
    const memory_info_t &mi = *it;
    // startEA endEA name sclass sbase bitness perm
    PyList_SetItem(py_list, i,
      Py_BuildValue("(" PY_FMT64 PY_FMT64 "ss" PY_FMT64 "II)",
        pyul_t(mi.startEA),
        pyul_t(mi.endEA),
        mi.name.c_str(),
        mi.sclass.c_str(),
        pyul_t(mi.sbase),
        (unsigned int)(mi.bitness),
        (unsigned int)mi.perm));
  }
  return py_list;
}

//-------------------------------------------------------------------------
PyObject *py_appcall(
  ea_t func_ea,
  thid_t tid,
  PyObject *py_type,
  PyObject *py_fields,
  PyObject *arg_list)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  if ( !PyList_Check(arg_list) )
    return NULL;

  const char *type   = py_type == Py_None ? NULL : PyString_AS_STRING(py_type);
  const char *fields = py_fields == Py_None ? NULL : PyString_AS_STRING(py_fields);

  // Convert Python arguments into IDC values
  qvector<idc_value_t> idc_args;
  int sn = 0;
  Py_ssize_t nargs = PyList_Size(arg_list);
  idc_args.resize(nargs);
  bool ok = true;
  for ( Py_ssize_t i=0; i<nargs; i++ )
  {
    // Get argument
    borref_t py_item(PyList_GetItem(arg_list, i));
    if ( (debug & IDA_DEBUG_APPCALL) != 0 )
    {
      qstring s;
      PyW_ObjectToString(py_item.o, &s);
      msg("obj[%d]->%s\n", int(i), s.c_str());
    }
    // Convert it
    if ( pyvar_to_idcvar(py_item, &idc_args[i], &sn) < CIP_OK )
    {
      ok = false;
      break;
    }
  }

  // Set exception message
  if ( !ok )
  {
    PyErr_SetString(
        PyExc_ValueError,
        "PyAppCall: Failed to convert Python values to IDC values");
    return NULL;
  }

  error_t ret;
  idc_value_t idc_result;
  Py_BEGIN_ALLOW_THREADS;

  if ( (debug & IDA_DEBUG_APPCALL) != 0 )
  {
    msg("input variables:\n"
        "----------------\n");

    qstring s;
    for ( Py_ssize_t i=0; i<nargs; i++ )
    {
      VarPrint(&s, &idc_args[i]);
      msg("%d]\n%s\n-----------\n", int(i), s.c_str());
      s.qclear();
    }
  }

  // Do Appcall
  ret = appcall(
    func_ea,
    tid,
    (type_t *)type,
    (p_list *)fields,
    idc_args.size(),
    idc_args.begin(),
    &idc_result);

  Py_END_ALLOW_THREADS;

  if ( ret != eOk )
  {
    // An exception was thrown?
    if ( ret == eExecThrow )
    {
      // Convert the result (which is a debug_event) into a Python object
      ref_t py_appcall_exc;
      idcvar_to_pyvar(idc_result, &py_appcall_exc);
      PyErr_SetObject(PyExc_OSError, py_appcall_exc.o);
      return NULL;
    }
    // An error in the Appcall? (or an exception but AppCallOptions/DEBEV is not set)
    else
    {
      char err_str[MAXSTR];
      qstrerror(ret, err_str, sizeof(err_str));
      PyErr_SetString(PyExc_Exception, err_str);
      return NULL;
    }
  }

  if ( (debug & IDA_DEBUG_APPCALL) != 0 )
  {
    msg("return variables:\n"
        "-----------------\n");
    qstring s;
    for ( Py_ssize_t i=0; i<nargs; i++ )
    {
      VarPrint(&s, &idc_args[i]);
      msg("%d]\n%s\n-----------\n", int(i), s.c_str());
      s.qclear();
    }
  }

  // Convert IDC values back to Python values
  for ( Py_ssize_t i=0; i<nargs; i++ )
  {
    // Get argument
    borref_t py_item(PyList_GetItem(arg_list, i));
    // We convert arguments but fail only on fatal errors
    // (we ignore failure because of immutable objects)
    if ( idcvar_to_pyvar(idc_args[i], &py_item) == CIP_FAILED )
    {
      PyErr_SetString(PyExc_ValueError, "PyAppCall: Failed while converting IDC values to Python values");
      return NULL;
    }
  }
  // Convert the result from IDC back to Python
  ref_t py_result;
  if ( idcvar_to_pyvar(idc_result, &py_result) <= CIP_IMMUTABLE )
  {
    PyErr_SetString(PyExc_ValueError, "PyAppCall: Failed while converting IDC return value to Python return value");
    return NULL;
  }
  QASSERT(30413, py_result.o->ob_refcnt == 1);
  if ( (debug & IDA_DEBUG_APPCALL) != 0 )
  {
    msg("return var:\n"
        "-----------\n");
    qstring s;
    VarPrint(&s, &idc_result);
    msg("%s\n-----------\n", s.c_str());
  }
  py_result.incref();
  return py_result.o;
}
//</code(py_idd)>


//-------------------------------------------------------------------------
//<inline(py_idd)>
/*
#<pydoc>
def dbg_get_registers():
    """
    This function returns the register definition from the currently loaded debugger.
    Basically, it returns an array of structure similar to to idd.hpp / register_info_t
    @return:
        None if no debugger is loaded
        tuple(name, flags, class, dtyp, bit_strings, default_bit_strings_mask)
        The bit_strings can be a tuple of strings or None (if the register does not have bit_strings)
    """
    pass
#</pydoc>
*/
static PyObject *dbg_get_registers()
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  if ( dbg == NULL )
    Py_RETURN_NONE;

  PyObject *py_list = PyList_New(dbg->registers_size);

  for ( int i=0; i<dbg->registers_size; i++ )
  {
    register_info_t &ri = dbg->registers(i);
    PyObject *py_bits;

    // Does this register have bit strings?
    // (Make sure it does not use custom formats because bit_string would be the format name)
    if ( ri.bit_strings != NULL && (ri.flags & REGISTER_CUSTFMT) == 0 )
    {
      int nbits = (int)b2a_width((int)get_dtyp_size(ri.dtyp), 0) * 4;
      py_bits = PyList_New(nbits);
      for ( int i=0; i<nbits; i++ )
      {
        const char *s = ri.bit_strings[i];
        PyList_SetItem(py_bits, i, PyString_FromString(s == NULL ? "" : s));
      }
    }
    else
    {
      Py_INCREF(Py_None);
      py_bits = Py_None;
    }

    // name, flags, class, dtyp, bit_strings, default_bit_strings_mask
    PyList_SetItem(py_list, i,
      Py_BuildValue("(sIIINI)",
        ri.name,
        ri.flags,
        (unsigned int)ri.register_class,
        (unsigned int)ri.dtyp,
        py_bits,
        (unsigned int)ri.default_bit_strings_mask));
  }
  return py_list;
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def dbg_get_thread_sreg_base(tid, sreg_value):
    """
    Returns the segment register base value
    @param tid: thread id
    @param sreg_value: segment register (selector) value
    @return:
        - The base as an 'ea'
        - Or None on failure
    """
    pass
#</pydoc>
*/
static PyObject *dbg_get_thread_sreg_base(PyObject *py_tid, PyObject *py_sreg_value)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  if ( !dbg_can_query() || !PyInt_Check(py_tid) || !PyInt_Check(py_sreg_value) )
    Py_RETURN_NONE;
  ea_t answer;
  thid_t tid = PyInt_AsLong(py_tid);
  int sreg_value = PyInt_AsLong(py_sreg_value);
  if ( internal_get_sreg_base(tid, sreg_value, &answer) != 1 )
    Py_RETURN_NONE;

  return Py_BuildValue(PY_FMT64, pyul_t(answer));
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def dbg_read_memory(ea, sz):
    """
    Reads from the debugee's memory at the specified ea
    @return:
        - The read buffer (as a string)
        - Or None on failure
    """
    pass
#</pydoc>
*/
static PyObject *dbg_read_memory(PyObject *py_ea, PyObject *py_sz)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  uint64 ea, sz;
  if ( !dbg_can_query() || !PyW_GetNumber(py_ea, &ea) || !PyW_GetNumber(py_sz, &sz) )
    Py_RETURN_NONE;

  // Create a Python string
  PyObject *ret = PyString_FromStringAndSize(NULL, Py_ssize_t(sz));
  if ( ret == NULL )
    Py_RETURN_NONE;

  // Get the internal buffer
  Py_ssize_t len;
  char *buf;
  PyString_AsStringAndSize(ret, &buf, &len);

  if ( (size_t)read_dbg_memory(ea_t(ea), buf, size_t(sz)) != sz )
  {
    // Release the string on failure
    Py_DECREF(ret);
    // Return None on failure
    Py_RETURN_NONE;
  }
  return ret;
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def dbg_write_memory(ea, buffer):
    """
    Writes a buffer to the debugee's memory
    @return: Boolean
    """
    pass
#</pydoc>
*/
static PyObject *dbg_write_memory(PyObject *py_ea, PyObject *py_buf)
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  uint64 ea;
  if ( !dbg_can_query() || !PyString_Check(py_buf) || !PyW_GetNumber(py_ea, &ea) )
    Py_RETURN_NONE;

  size_t sz = PyString_GET_SIZE(py_buf);
  void *buf = (void *)PyString_AS_STRING(py_buf);
  if ( write_dbg_memory(ea_t(ea), buf, sz) != sz )
    Py_RETURN_FALSE;
  Py_RETURN_TRUE;
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def dbg_get_name():
    """
    This function returns the current debugger's name.
    @return: Debugger name or None if no debugger is active
    """
    pass
#</pydoc>
*/
static PyObject *dbg_get_name()
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  if ( dbg == NULL )
    Py_RETURN_NONE;
  else
    return PyString_FromString(dbg->name);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def dbg_get_memory_info():
    """
    This function returns the memory configuration of a debugged process.
    @return:
        None if no debugger is active
        tuple(startEA, endEA, name, sclass, sbase, bitness, perm)
    """
    pass
#</pydoc>
*/
static PyObject *dbg_get_memory_info()
{
  PYW_GIL_CHECK_LOCKED_SCOPE();

  if ( !dbg_can_query() )
    Py_RETURN_NONE;

  // Invalidate memory
  meminfo_vec_t areas;
  Py_BEGIN_ALLOW_THREADS;
  invalidate_dbgmem_config();
  invalidate_dbgmem_contents(BADADDR, BADADDR);

  get_dbg_memory_info(&areas);
  Py_END_ALLOW_THREADS;
  return meminfo_vec_t_to_py(areas);
}

//-------------------------------------------------------------------------
/*
#<pydoc>
def dbg_can_query():
    """
    This function can be used to check if the debugger can be queried:
      - debugger is loaded
      - process is suspended
      - process is not suspended but can take requests. In this case some requests like
        memory read/write, bpt management succeed and register querying will fail.
        Check if idaapi.get_process_state() < 0 to tell if the process is suspended
    @return: Boolean
    """
    pass
#</pydoc>
*/
static bool dbg_can_query();
PyObject *py_appcall(
  ea_t func_ea,
  thid_t tid,
  PyObject *py_type,
  PyObject *py_fields,
  PyObject *arg_list);

char get_event_module_name(const debug_event_t *ev, char *buf, size_t bufsize)
{
    qstrncpy(buf, ev->modinfo.name, bufsize);
    return true;
}

ea_t get_event_module_base(const debug_event_t *ev)
{
    return ev->modinfo.base;
}

asize_t get_event_module_size(const debug_event_t *ev)
{
    return ev->modinfo.size;
}

char get_event_exc_info(const debug_event_t *ev, char *buf, size_t bufsize)
{
    qstrncpy(buf, ev->exc.info, bufsize);
    return true;
}

char get_event_info(const debug_event_t *ev, char *buf, size_t bufsize)
{
    qstrncpy(buf, ev->info, bufsize);
    return true;
}

ea_t get_event_bpt_hea(const debug_event_t *ev)
{
    return ev->bpt.hea;
}

uint get_event_exc_code(const debug_event_t *ev)
{
    return ev->exc.code;
}

ea_t get_event_exc_ea(const debug_event_t *ev)
{
    return ev->exc.ea;
}

bool can_exc_continue(const debug_event_t *ev)
{
    return ev->exc.can_cont;
}

//</inline(py_idd)>
