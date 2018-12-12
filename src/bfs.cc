/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: lishen chen <frankchenls@outlook.com>                                                            |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_bfs.h"
#include "bfs_c.h"
/* If you declare any globals in php_bfs.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(bfs);

 struct bfs_object {
    char * bfs_status_t;
    char * read_buf;
    bfs_fs_t *fs;
    bfs_file_t *file;
    zend_object std;
};
/* True global resources - no need for thread safety here */
static int le_bfs;
zend_class_entry *bfs_ce;
zend_object_handlers bfs_object_handlers;


/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("bfs.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_bfs_globals, bfs_globals)
    STD_PHP_INI_ENTRY("bfs.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_bfs_globals, bfs_globals)
PHP_INI_END()
*/
/* }}} */


static inline bfs_object *bfs_fetch_object(zend_object *obj) /* {{{ */ {
        return (bfs_object *)((char*)(obj) - XtOffsetOf(bfs_object, std));
}

void bfs_object_free_storage(zend_object *object)
{
    bfs_object *intern = bfs_fetch_object(object);
    zend_object_std_dtor(&intern->std);
}

static  zend_object* bfs_object_create(zend_class_entry *type TSRMLS_DC)
{

    bfs_object *obj = (bfs_object *)emalloc(sizeof(bfs_object)+ zend_object_properties_size(type));
     zend_object_std_init(&obj->std, type);
      object_properties_init(&obj->std, type);
     obj->std.handlers = &bfs_object_handlers;

    return &obj->std;
}


const zend_function_entry bfs_methods[] = {
    PHP_ME(BFS,  __construct,     NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(BFS,  __destruct,     NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(BFS,  ls,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  init,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  touchz,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  fopen,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  fclose,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  fread,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  fwrite,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  fseek,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  mkdir,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  rmdir,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  chmod,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  put,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  get,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  rename,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  remove,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  du,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  location,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  cat,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  status,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS,  symlink,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(BFS, changeReplicaNum,      NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

PHP_METHOD(BFS, __construct)
{
	RETURN_TRUE;
}

PHP_METHOD(BFS, __destruct)
{
    zval  *self = getThis();
    bfs_object *obj = bfs_fetch_object(Z_OBJ_P((self)));
    efree(obj->read_buf);
    RETURN_TRUE;
}
PHP_METHOD(BFS, init)
{
	zend_string *flag_file_path;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &flag_file_path) == FAILURE)	{
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(flag_file_path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
        obj->fs =bfs_open_file_system(ZSTR_VAL(flag_file_path));
        obj->file=NULL;
        obj->bfs_status_t=NULL;
        obj->read_buf=NULL;
    if(obj->fs==NULL)
        RETURN_FALSE;
  	 RETURN_TRUE;
}

PHP_METHOD(BFS, touchz)
{
    zend_string *path;
    long int result;
    #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }    
     #else
     ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
    zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
    result=bfs_touchz(obj->fs,ZSTR_VAL(path));
    RETURN_LONG(result);
        
}

PHP_METHOD(BFS, fopen)
{
    zend_string *path;
    zend_string *mode;
    long int result;
    #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &path,&mode) == FAILURE) {
        return;
       }    
     #else
     ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(path)
        Z_PARAM_STR(mode)
        ZEND_PARSE_PARAMETERS_END();
     #endif
    zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
    if(strcmp(ZSTR_VAL(mode),"w")==0)
        obj->file=bfs_open_file(obj->fs,ZSTR_VAL(path),O_WRONLY);
    if(strcmp(ZSTR_VAL(mode),"r")==0)
        obj->file=bfs_open_file(obj->fs,ZSTR_VAL(path),O_RDONLY);
    if(obj->file==NULL)
        RETURN_FALSE;
     RETURN_TRUE;        
}

PHP_METHOD(BFS, fclose)
{             
    long int ret;       
    zval *object = getThis();
    bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
        ret=bfs_close_file(obj->file);
      RETURN_LONG(ret);       
}

PHP_METHOD(BFS,fread)
{
    long int len;
    long int result;
    #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l",&len) == FAILURE) {
        return;
       }    
     #else
     ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_LONG(len)
        ZEND_PARSE_PARAMETERS_END();
     #endif
    zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
    if((obj->read_buf=(char*)erealloc(obj->read_buf,len+1))==NULL)
        RETURN_NULL();
    memset(obj->read_buf,0,len+1);
    result=bfs_read_file(obj->file,obj->read_buf,len);
    if(obj->read_buf==NULL)
        RETURN_LONG(result);
    RETURN_STRING(obj->read_buf);       
}
PHP_METHOD(BFS,fwrite)
{
    zend_string *buf;
    long int result;
    #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S",&buf) == FAILURE) {
        return;
       }    
     #else
     ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(buf)
    ZEND_PARSE_PARAMETERS_END();
     #endif
    zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
    result=bfs_write_file(obj->file,ZSTR_VAL(buf),ZSTR_LEN(buf));
    RETURN_LONG(result);   
}

PHP_METHOD(BFS,fseek)
{
    long int offset;
    long int whence;
    long int result;
    #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l",&offset,&whence) == FAILURE) {
        return;
       }    
     #else
     ZEND_PARSE_PARAMETERS_START(1,2)
        Z_PARAM_LONG(offset)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(whence)
    ZEND_PARSE_PARAMETERS_END();
     #endif
    zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
    result=bfs_seek(obj->file,offset,whence);
    RETURN_LONG(result);   
}
PHP_METHOD(BFS, ls)
{
	zend_string *path;
	long int result;
	#ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }    
     #else
   	 ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
    	ZEND_PARSE_PARAMETERS_END();
     #endif
	zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	if(obj->fs==NULL) RETURN_NULL();
	result=bfs_list_directory(obj->fs,ZSTR_VAL(path));
	RETURN_LONG(result);
        
}
PHP_METHOD(BFS, mkdir)
{
        zend_string *path;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_create_directory(obj->fs,ZSTR_VAL(path));
         RETURN_LONG(result);
}
PHP_METHOD(BFS, rmdir)
{
        zend_string *path;
        zend_bool recursive;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sb", &path,&recursive) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(path)
        Z_PARAM_BOOL(recursive)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_rm_dir(obj->fs, ZSTR_VAL(path), recursive);
         RETURN_LONG(result);
}
PHP_METHOD(BFS, chmod)
{
        zend_string *mod;
        zend_string *path;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS",&mod,&path) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(2, 2)
         Z_PARAM_STR(mod)
         Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_chmod(obj->fs, ZSTR_VAL(mod),  ZSTR_VAL(path));
         RETURN_LONG(result);
}
PHP_METHOD(BFS, put)
{
	zend_string *local;
	zend_string *bfs;
	long int result;
	#ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &local,&bfs) == FAILURE) {
        return;
       }
     #else
   	 ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(local)
        Z_PARAM_STR(bfs)
    	ZEND_PARSE_PARAMETERS_END();
     #endif
	zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	if(obj->fs==NULL) RETURN_NULL();
	result=bfs_put(obj->fs,ZSTR_VAL(local),ZSTR_VAL(bfs));
	RETURN_LONG(result);

}
PHP_METHOD(BFS, get)
{
	zend_string *bfs;
	zend_string *local;
	long int result;
	#ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &bfs,&local) == FAILURE) {
        return;
       }
     #else
   	 ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(bfs)
        Z_PARAM_STR(local)
    	ZEND_PARSE_PARAMETERS_END();
     #endif
	zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	if(obj->fs==NULL) RETURN_NULL();
	result=bfs_get(obj->fs,ZSTR_VAL(bfs),ZSTR_VAL(local));
	RETURN_LONG(result);

}
PHP_METHOD(BFS, rename)
{
        zend_string *oldpath;
        zend_string *newpath;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS",&oldpath,&newpath) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(2, 2)
         Z_PARAM_STR(oldpath)
         Z_PARAM_STR(newpath)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_rename(obj->fs, ZSTR_VAL(oldpath),  ZSTR_VAL(newpath));
         RETURN_LONG(result);
}
PHP_METHOD(BFS, remove)
{
        zend_string *path;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_delete_file(obj->fs, ZSTR_VAL(path));
         RETURN_LONG(result);
}
PHP_METHOD(BFS, du)
{
        zend_string *path;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_du_v2(obj->fs, ZSTR_VAL(path));
         RETURN_LONG(result);
}
PHP_METHOD(BFS, location)
{
        zend_string *path;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_location(obj->fs, ZSTR_VAL(path));
         RETURN_LONG(result);
}

PHP_METHOD(BFS, cat)
{
        zend_string *path;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &path) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(1, 1)
        Z_PARAM_STR(path)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
	 if(obj->fs==NULL) RETURN_NULL();
        result=bfs_cat(obj->fs, ZSTR_VAL(path));
         RETURN_LONG(result);
}
PHP_METHOD(BFS, status)
{          
        long int ret;        
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
     if(obj->fs==NULL) RETURN_NULL();
        ret=bfs_status(obj->fs, obj->bfs_status_t);
        if(obj->bfs_status_t==NULL)
            RETURN_LONG(ret);
       RETURN_STRING(obj->bfs_status_t);
        
}

PHP_METHOD(BFS, symlink)
{
    zend_string *src;
    zend_string *dst;
    long int result;
    #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &src,&dst) == FAILURE) {
        return;
       }
     #else
     ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_STR(src)
        Z_PARAM_STR(dst)
        ZEND_PARSE_PARAMETERS_END();
     #endif
    zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
    if(obj->fs==NULL) RETURN_NULL();
    result=bfs_symlink(obj->fs,ZSTR_VAL(src),ZSTR_VAL(dst));
    RETURN_LONG(result);

}
PHP_METHOD(BFS,changeReplicaNum)
{
        zend_string *path;
        zend_string *replica_num;
        long int result;
        #ifndef FAST_ZPP
    /* Get function parameters and do error-checking. */
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS",&path,&replica_num) == FAILURE) {
        return;
       }
     #else
         ZEND_PARSE_PARAMETERS_START(2, 2)
         Z_PARAM_STR(path)
         Z_PARAM_STR(replica_num)
        ZEND_PARSE_PARAMETERS_END();
     #endif
        zval *object = getThis();
        bfs_object *obj = bfs_fetch_object(Z_OBJ_P((object)));
     if(obj->fs==NULL) RETURN_NULL();
        result=bfs_change_replica_num(obj->fs, ZSTR_VAL(path),  ZSTR_VAL(replica_num));
         RETURN_LONG(result);
}

PHP_MINIT_FUNCTION(bfs)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	zend_class_entry ce;
   	INIT_CLASS_ENTRY(ce, "BFS", bfs_methods);
        bfs_ce = zend_register_internal_class(&ce);
        bfs_ce->create_object = bfs_object_create;
        memcpy(&bfs_object_handlers,
        zend_get_std_object_handlers(), sizeof(zend_object_handlers));
        bfs_object_handlers.clone_obj = NULL;
	bfs_object_handlers.offset = XtOffsetOf(bfs_object, std);
	bfs_object_handlers.free_obj = bfs_object_free_storage;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(bfs)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(bfs)
{
#if defined(COMPILE_DL_BFS) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(bfs)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(bfs)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "bfs support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}


/* {{{ bfs_module_entry
 */
zend_module_entry bfs_module_entry = {
	STANDARD_MODULE_HEADER,
	"bfs",
	NULL,
	PHP_MINIT(bfs),
	PHP_MSHUTDOWN(bfs),
	PHP_RINIT(bfs),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(bfs),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(bfs),
	PHP_BFS_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_BFS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(bfs)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */  
