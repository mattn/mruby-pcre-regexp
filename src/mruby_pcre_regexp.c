#include <stdio.h>
#include <memory.h>
#include <mruby.h>
#include <mruby/class.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/data.h>
#include <mruby/variable.h>
#if defined(_WIN32) || defined(_WIN64)
#include <shlwapi.h>
  #define STRCHR StrChr
#else
  #define STRCHR strchr
#endif
#include "pcre.h"

struct mrb_pcre_regexp {
  pcre* re;
  int flag;
};

static void
pcre_regexp_free(mrb_state *mrb, void *p) {
  struct mrb_pcre_regexp *pre = (struct mrb_pcre_regexp *) p;
  pcre_free(pre->re);
  mrb_free(mrb, pre);
}

static struct mrb_data_type mrb_pcre_regexp_type = {
  "PosixRegexp", pcre_regexp_free
};

static void
pcre_regexp_init(mrb_state *mrb, mrb_value self, mrb_value str, mrb_value flag) {
  mrb_value regexp;
  struct mrb_pcre_regexp *reg;
  int cflag = 0;
  int erroff = 0;
  const char *errstr = NULL;

  regexp = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@regexp"));
  if (mrb_nil_p(regexp)) {
    reg = malloc(sizeof(struct mrb_pcre_regexp));
    memset(reg, 0, sizeof(struct mrb_pcre_regexp));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@regexp"), mrb_obj_value(
        Data_Wrap_Struct(mrb, mrb->object_class,
          &mrb_pcre_regexp_type, (void*) reg)));
  }else{
    Data_Get_Struct(mrb, regexp, &mrb_pcre_regexp_type, reg);
    pcre_free(reg->re);
  }

  if (mrb_nil_p(flag))
    cflag = 0;
  else if (mrb_fixnum_p(flag)) {
    int nflag = mrb_fixnum(flag);
    if (nflag & 1) cflag |= PCRE_CASELESS;
    if (nflag & 2) cflag |= PCRE_EXTENDED;
    if (nflag & 4) cflag |= PCRE_MULTILINE | PCRE_DOTALL;
  } else if (mrb_type(flag) == MRB_TT_TRUE)
    cflag |= PCRE_CASELESS;
  else if (mrb_string_p(flag)) {
    if (STRCHR(RSTRING_PTR(flag), 'i')) cflag |= PCRE_CASELESS;
    if (STRCHR(RSTRING_PTR(flag), 'x')) cflag |= PCRE_EXTENDED;
    if (STRCHR(RSTRING_PTR(flag), 'm')) cflag |= PCRE_MULTILINE | PCRE_DOTALL;
  }
  reg->flag = cflag;
  reg->re = pcre_compile(RSTRING_PTR(str), cflag, &errstr, &erroff, NULL);
  if (!reg->re) {
    mrb_raisef(mrb, E_ARGUMENT_ERROR, "'%S' is an invalid regular expression because %S.",
      mrb_str_new_cstr(mrb, RSTRING_PTR(str) + erroff), mrb_str_new_cstr(mrb, errstr));
  }
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@source"), str);
}

static mrb_value
pcre_regexp_initialize(mrb_state *mrb, mrb_value self) {
  mrb_value source, flag = mrb_nil_value();

  mrb_get_args(mrb, "S|o", &source, &flag);
  pcre_regexp_init(mrb, self, source, flag);
  return mrb_nil_value();
}

static mrb_value
pcre_regexp_initialize_copy(mrb_state *mrb, mrb_value copy) {
  mrb_value regexp;
  struct mrb_pcre_regexp *reg;

  mrb_get_args(mrb, "o", &regexp);
  if (mrb_obj_equal(mrb, copy, regexp)){
    return copy;
  }
  if (!mrb_obj_is_instance_of(mrb, regexp, mrb_obj_class(mrb, copy))){
    mrb_raise(mrb, E_TYPE_ERROR, "wrong argument class");
  }

  Data_Get_Struct(mrb, regexp, &mrb_pcre_regexp_type, reg);
  pcre_regexp_init(mrb, copy, mrb_funcall_argv(mrb, regexp, mrb_intern_lit(mrb, "source"), 0, NULL), mrb_fixnum_value(reg->flag));
  return copy;
}
static mrb_value
pcre_regexp_match(mrb_state *mrb, mrb_value self) {
  const char *str;
  char global_match[3];
  mrb_value regexp;
  struct mrb_pcre_regexp *reg;
  int i;
  mrb_value mrb_i, mrb_match;
  size_t nmatch = 999;
  int match[999];
  int regno;
  int ai;
  struct RClass* clazz;
  mrb_value c;
  mrb_value args[2];
  
  mrb_get_args(mrb, "z", &str);

  regexp = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp, &mrb_pcre_regexp_type, reg);

  regno = pcre_exec(reg->re, NULL, str, strlen(str), 0, 0, match, nmatch);
  if (regno < 0)
    return mrb_nil_value();

  mrb_obj_iv_set(mrb, (struct RObject *)mrb_class_real(RDATA(self)->c), mrb_intern_lit(mrb, "@last_match"), mrb_nil_value());

  ai = mrb_gc_arena_save(mrb);
  clazz = mrb_class_get(mrb, "PcreMatchData");
  c = mrb_obj_new(mrb, clazz, 0, NULL);
  mrb_iv_set(mrb, c,mrb_intern_lit(mrb, "@string"), mrb_str_new_cstr(mrb, str));
  
  for (i = 0; i < regno; i++) {
    args[0] = mrb_fixnum_value(match[i * 2]);
    args[1] = mrb_fixnum_value(match[i * 2 + 1] - match[i * 2]);
    mrb_funcall_argv(mrb, c, mrb_intern_lit(mrb, "push"), sizeof(args)/sizeof(args[0]), &args[0]);
    if (i > 0 && i < 10) {
      sprintf(global_match, "$%i", i);
      mrb_i = mrb_fixnum_value(i);
      mrb_match = mrb_funcall_argv(mrb, c, mrb_intern_lit(mrb, "[]"), 1, &mrb_i);
      mrb_gv_set(mrb, mrb_intern_cstr(mrb, global_match), mrb_match);
    }
    mrb_gc_arena_restore(mrb, ai);
  }

  mrb_obj_iv_set(mrb, (struct RObject *)mrb_class_real(RDATA(self)->c), mrb_intern_lit(mrb, "@last_match"), c);
  return c;
}

static mrb_value
pcre_regexp_equal(mrb_state *mrb, mrb_value self) {
  mrb_value other, regexp_self, regexp_other;
  struct mrb_pcre_regexp *self_reg, *other_reg;

  mrb_get_args(mrb, "o", &other);
  if (mrb_obj_equal(mrb, self, other)){
    return mrb_true_value();
  }
  if (mrb_nil_p(other)) {
    return mrb_false_value();
  }
  regexp_self = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@regexp"));
  regexp_other = mrb_iv_get(mrb, other, mrb_intern_lit(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp_self, &mrb_pcre_regexp_type, self_reg);
  Data_Get_Struct(mrb, regexp_other, &mrb_pcre_regexp_type, other_reg);

  if (!self_reg || !other_reg){
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid PcreRegexp");
  }
  if (self_reg->flag != other_reg->flag){
      return mrb_false_value();
  }
  return mrb_str_equal(mrb, mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@source")), mrb_iv_get(mrb, other, mrb_intern_lit(mrb, "@source"))) ?
      mrb_true_value() : mrb_false_value();
}

static mrb_value
pcre_regexp_casefold_p(mrb_state *mrb, mrb_value self) {
  mrb_value regexp;
  struct mrb_pcre_regexp *reg;

  regexp = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@regexp"));
  Data_Get_Struct(mrb, regexp, &mrb_pcre_regexp_type, reg);
  return (reg->flag & PCRE_CASELESS) ? mrb_true_value() : mrb_false_value();
}

void
mrb_mruby_pcre_regexp_gem_init(mrb_state* mrb) {
  struct RClass *clazz;

  clazz = mrb_define_class(mrb, "PcreRegexp", mrb->object_class);

  mrb_define_const(mrb, clazz, "IGNORECASE", mrb_fixnum_value(1));
  mrb_define_const(mrb, clazz, "EXTENDED", mrb_fixnum_value(2));
  mrb_define_const(mrb, clazz, "MULTILINE", mrb_fixnum_value(4));

  mrb_define_method(mrb, clazz, "initialize", pcre_regexp_initialize, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(2));
  mrb_define_method(mrb, clazz, "initialize_copy", pcre_regexp_initialize_copy, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "==", pcre_regexp_equal, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "match", pcre_regexp_match, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, clazz, "casefold?", pcre_regexp_casefold_p, MRB_ARGS_NONE());
}

void
mrb_mruby_pcre_regexp_gem_final(mrb_state* mrb) {
}

// vim:set et:
