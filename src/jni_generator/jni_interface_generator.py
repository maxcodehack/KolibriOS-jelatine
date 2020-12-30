import os,sys
from str_code import *


primitive_types=['byte',"boolean","short","int","char","long","float","double"]
primitive_signatures=['B',"Z","","I","C","J","F","D"]
is_primitive_type_last_index=-1
NO_ERRORS=1
def is_primitive_type(s_typ):
	ix=-1
	global is_primitive_type_last_index
	is_primitive_type_last_index=-1
	for s in primitive_types:
		ix+=1
		if (s_typ==s):
			is_primitive_type_last_index=ix
			return 1
	return 0
def get_signature_type(typ):
	if (is_primitive_type(typ)):
		return primitive_signatures[is_primitive_type_last_index]
	elif (typ=="String"):
		return "Ljava/lang/String;"
	elif (typ=="Object"):
		return "Ljava/lang/Object;"
	elif (typ=="Class"):
		return "Ljava/lang/Class;"
	elif (typ=="Thread"):
		return "Ljava/lang/Thread;"
	elif (typ=="int[]"):
		return "[I"
	elif (typ=="byte[]"):
		return "[B"
	elif (typ=="long[]"):
		return "[J"
	elif (typ=="float[]"):
		return "[F"
	elif (typ=="double[]"):
		return "[D"
	elif (typ=="char[]"):
		return "[C"
	return "L"+typ+";"

def append_signature_type(s,typ):
	sig=get_signature_type(typ)
	le_sig=len(sig)
	if le_sig==1:
		s+=sig
	elif (le_sig)==2:
		if sig[0]=='[':
			s+=sig
		else:
			s+=sig
		return s
	else:
		s+=sig#as class
	return s;
	
	
		
		
			
class FuncParam:
	def __init__(self,typ,name):
		self.typ=typ
		self.name=name
	def __str__(self):
		s="Param={ "+self.typ+" "+self.name+"}, "
		return s
	def toString(self):
		s=self.typ+" "+self.name
		return s
	def getType(self):
		return self.typ
	def getName(self):
		return self.name
class ParamList:
	def __init__(self):
		self.list=[]
	def __str__(self):
		s=""
		i=-1;
def exits_simple_mangled_name(mangled_name,candidates=[],excludeIx=-1):
	pass	
class JNIFunc:
	def __init__(self,class_,name,orig_name):
		self.class_name=class_
		self.orig=orig_name
		self.public=0
		self.static=0
		self.final=0
		self.protected=0
		self.native=1
		self.resultType=''
		self.params=[]
		self.synchronized =0
		self.name=name
		self.specJNIName=None
	def __str__(self):
		s="JNIFunc={ "
		if (self.static):
			s+=" static"
		s+=" "+self.resultType
		s+=" "+self.class_name+"::"
		s+=" "+self.name

		s+=" ("
		if len(self.params)!=0:
			#s+=str(self.params)
			s+=self.getJavaParamList()
		s+=");}"
		s+="\n//"+self.orig
		return s
	def addParam(self,param):
		self.params.add(param)
	def getJavaParamList(self):
		s=""
		i=-1
		hi= len(self.params)-1
		for param in self.params:
			i+=1
			#s+=str(param)
			s+=param.toString()
			if (i<hi):
				s+=","
		return s
	def getJelatineCFuncname(self):
		classname=self.class_name.replace('/','_')
		full_func_name=classname+"_"+self.name 
		return full_func_name
	def toJelatineJNIInterface(self,NoStaticCMod=0):
		kni_ret_type=self.getJNI_C_return_type()
		comment="//"+str(self)+"\n"
		s=""
		if(NoStaticCMod==0):
			s+="static "
		s+=kni_ret_type+" "
		full_func_name=self.getJelatineCFuncname()
		s+=full_func_name+"(void)"
		func_decl=comment+"extern "+s
		s=comment+s
		s+="\n{"
		startDef,num_handles,has_end_handles_in_end_def,endDef=self.getJNIReturnTypeValueDefinition()
		primitive_defs,handles_defs,get_actions=self.getParamHandlesDefs()
		middleDef=''
		if (num_handles==0):#primitive type
			if len(startDef)>0:
				s+="\n\t"+startDef
			startDef=''
		if len(primitive_defs)>0:
			s+=primitive_defs
		if not(self.static):
			num_handles+=1
			startDef+="KNI_DeclareHandle(this_ref);\n\t"
			middleDef="KNI_GetThisPointer(this_ref);"
		num_handles+=len(handles_defs)
		if (num_handles>0):
			s+="\n\tKNI_StartHandles("+str(num_handles)+");"

		if len(startDef)>0:
			s+="\n\t"+startDef
		for hand_def in handles_defs:
			s+="\n\t"+hand_def
		if len(middleDef)>0:
			s+="\n\n\t"+middleDef
		if len(get_actions):
			s+=get_actions
		s+="\n\n\tfprintf(stdout,\"\\nIn "+full_func_name+"!\");"
		if (num_handles>0) and not has_end_handles_in_end_def:
			s+="\n\tKNI_EndHandles();"
		if len(endDef)>0:
			s+="\n\t"+endDef
		s+="\n}\n"
		return (s,func_decl);
	def getJNI_C_return_type(self):
		kni_predef='KNI_RETURNTYPE_'
		rt=self.resultType
		if (rt=="int[]"):
			return kni_predef+"OBJECT"
		elif (rt=="byte"):
			return kni_predef+"OBJECT"
		elif (rt=="Thread"):
			return kni_predef+"OBJECT"
		
		ret_type=kni_predef+trim_str(self.resultType.upper())
		return ret_type
	def getParamHandlesDefs(self):
		primitive_defs=""
		handles_defs=[]
		get_actions=""
		i=-1
		hi= len(self.params)-1
		sp=1;
		
		for param in self.params:
			typ=param.getType()
			name=param.getName()
			if (is_primitive_type(typ)):
				decl="j"+typ+" "+name+";"
				primitive_defs+="\n\t"+decl
				typ2=typ[0].upper()+typ[1:]
				get_actions+="\n\t"+name+"=KNI_GetParameterAs"+typ2+"("+str(sp)+");"
				sp+=1
				if (typ=="double") or (typ=="long"):#64 bit value
					sp+=1;
			else:
				handles_defs.append("\n\tKNI_DeclareHandle("+name+");")
				get_actions+="\n\tKNI_GetParameterAsObject("+str(sp)+","+name+");"
				sp+=1
			#elif (typ==)
		return (primitive_defs,handles_defs,get_actions)	
	def getJNIReturnTypeValueDefinition(self):
		
		rt=self.resultType
		#print 'rt:'+rt+":"+str(len(rt))
		if (rt=="void"):
			return ("",0,0,"\n\tKNI_ReturnVoid();");
		if (is_primitive_type(rt)):
			rt2=rt[0].upper()+rt[1:]
			endDef="\n\tKNI_Return"+rt2+"(ret);\n"
			return ('j'+self.resultType+" ret;",0,0,endDef)
		if (rt=="String"):
			endDef="//KNI_NewStringUTF(str, str_ref);\n\tKNI_EndHandlesAndReturnObject(str_ref);"
			return ("char *str;KNI_DeclareHandle(str_ref);",1,1,endDef)
		if (rt=="Thread"):
			return ("KNI_DeclareHandle(thread_ref);",1,1,'KNI_EndHandlesAndReturnObject(thread_ref);')
		elif (rt=="byte[]"):
			#return ""
			return ("KNI_DeclareHandle(byte_array_ref);",1,1,'KNI_EndHandlesAndReturnObject(byte_array_ref);')
		elif (rt=="int[]"):
			return ("KNI_DeclareHandle(int_array_ref);",1,1,'KNI_EndHandlesAndReturnObject(int_array_ref);')
		if (rt=="Object"):
			endDef="//\n\tKNI_EndHandlesAndReturnObject(obj_ref);"
			return ("KNI_DeclareHandle(obj_ref);",1,1,endDef)
		return ("",0,0,"")
	
	def getSignatureDescriptor(self):
		r="("
		param_signature=""
		for param in self.params:
			param_signature=append_signature_type(param_signature,param.getType())
		r+=param_signature+')'
		if (self.resultType=='void'):
			r+='V'
		else:
			r+=get_signature_type(self.resultType)
		return r
	def toJelatineJNISignatureDescriptor(self):
		r="\n{"
		r+="\n\t\""+self.class_name+"\","
		r+="\n\t\""+self.name+"\","
		r+="\n\t\""+self.getSignatureDescriptor()+"\","
		r+="\n\t"+self.getJelatineCFuncname()+"\n},"
		return r;
		
		
	
	def parseParams(self,param_list):
		#lst=[]
		self.params=[]
		if (param_list==None):
			return
		for s3 in param_list:
			s3=s3.replace(']','] ')
			param_sp=splitBySpace(s3)
			
			if len(param_sp)<2:
				raise Exception("parseParams Error!len(param_sp)<2"+str(param_list))
			if (len(param_sp)>2):
				typ=""
				name=""
				i=-1
				hi=len(param_sp)-1
				i=hi
				found_name=0
				#for tp in param_sp:
				while (i>=0):
					tp=param_sp[i]
					i-=1
					if found_name:
						typ=tp+typ
					elif (tp=='[]'):
						typ=tp+typ
					else:
						name=tp
						found_name=1
			else:
				typ,name=param_sp[0],param_sp[1]
			
			find_d_brak=name.find('[]')
			
			while(find_d_brak>=0):
				typ+='[]'
				le=len(name)
				if find_d_brak==0:
					name=name[2:]
				else:
					name=name[0:le-2]
				find_d_brak=name.find('[]')
				
			f_param=FuncParam(typ,name)
			self.params.append(f_param)
	def getJavaStandartSimpleJNIName(self):
		pass
	def getJavaStandartJNIName(self,candidates=[],excludeIndex=-1):
		i=-1
		for cand in cand
		
def get_func_proto_name(prototype):
	fi_brak=prototype.find("(")
	if (fi_brak<0):
		return None
	sub_str=prototype[:fi_brak]
	sp=splitBySpace(sub_str)
	le=len(sp)
	return sp;#[le-1]
def get_param_list_part(prototype):
	fi_brak=prototype.find("(")
	fi_brak2=prototype.find(")")
	if (fi_brak<0) or (fi_brak2<0):
		print prototype
		print "\tNot correct prototype-method!"
		
		return None
	sub_str=trim_str(prototype[fi_brak+1:fi_brak2])
	if len(sub_str)==0:
		return None
	sp=sub_str.split(',')
	le=len(sp)
	return sp;#[le-1]
	
def load_jni_interfaces(src_data):
	#ret_list=[]
	funcs=[]
	for s in src_data:
		s2=trim_str(s)
		if (s2.find('#')==0) or (s2.find('//')==0) or (s2.find('*')==0):
			continue
		if len(s2)==0:
			continue
		f_dp=s2.find(':')
		if (f_dp<=0):
			raise Exception("Not found separator ':'")
		fi_end=s2.find(';')
		if (fi_end>0):
			s2=s2[:fi_end]
		sp_dp=s2.split(':')
		if len(sp_dp)<2:
			raise Exception("Error !len(sp_dp)!=2")
		full_class_name=trim_str(sp_dp[0])
		if full_class_name.startswith('./'):
			full_class_name=trim_str(full_class_name[2:])
		lo_s=full_class_name.lower()
		if (lo_s.endswith('.java')):
			le_full_class_name=len(full_class_name)
			full_class_name=full_class_name[:le_full_class_name-5]
		prototype=trim_str(sp_dp[1])
		if (prototype.find('//')==0)or (prototype.find('#')==0)or (prototype.find('*')==0):
			continue
		prototype_func_name_sp=get_func_proto_name(prototype)
		if (prototype_func_name_sp==None):
			if(NO_ERRORS):
				print 'skiped by error:'+s2
				continue
			else:
				raise Exception("Err!prototype_func_name_sp==None\nstr:"+s2)
		le_sp=len(prototype_func_name_sp)
		if (le_sp<2):
			raise Exception("Err!le_sp<2")
		prototype_func_name=prototype_func_name_sp[le_sp-1]
		func=JNIFunc(full_class_name,prototype_func_name,prototype)
		#sp=splitBySpace()
		in_param_list=0;
		is_done=1
		ix=-1
		typename=""
		for s3 in prototype_func_name_sp:
			ix+=1
			if ix==(le_sp-1):
				break
			if (s3=='public'):
				func.public=1
			elif (s3=='private'):
				func.public=0	
			elif (s3=='static'):
				func.static=1
			elif (s3=='native'):
				func.native=1
			elif (s3=='final'):
				func.final=1
			elif (s3=='protected'):
				func.protected=1
			elif (s3=="synchronized"):
				func.synchronized =1
			else:
				typename=typename+" " +s3
		func.resultType=trim_str(typename)
		sp=get_param_list_part(prototype)
		
		if (sp==None):
			#void param list
			funcs.append(func)
			continue
		func.parseParams(sp)
		funcs.append(func)
	return funcs
def gen_jelatine_jni_interfaces(funcs):
	r="""//Base of this source been was generated by jni_interfaces_generator.py

#include "wrappers.h"

#include "interpreter.h"
#include "jstring.h"
#include "kni.h"
#include "loader.h"
#include "native.h"
#include "thread.h"
#include "utf8_string.h"
#include "util.h"
#include "vm.h"

#include "java_lang_String.h"
#include "java_lang_Thread.h"

	"""
	jel_struct_native_declares=""
	funcs_rels=""
	funcs_defs=""
	for f in funcs:
		s,def_f= f.toJelatineJNIInterface(1)
		#r+="\n"+s
		funcs_rels+=s
		funcs_defs+="\n "+def_f+";"
		
		jel_struct_native_declares+="\n"+f.toJelatineJNISignatureDescriptor()
		#print s
	headers_defs=r
	return (headers_defs,funcs_defs,funcs_rels,jel_struct_native_declares)
def gen_jni_interface_to_file(src_fname,module_name=None,h_prefix="native_exts"):
	fi=open(src_fname)
	src_data=fi.readlines()
	fi.close()
	
	funcs=load_jni_interfaces(src_data)
	if 0:
		for f in funcs:
			print str(f)
	headers_defs,funcs_defs,funcs_rels,jel_struct_native_declares=gen_jelatine_jni_interfaces(funcs)
	dirName=extract_dir(src_fname)
	shortName=extract_fname(src_fname)
	if (module_name==None):
		module_name=change_ext(shortName,'c')
	if (module_name.find('.c')<0):
		module_name+=".c"
	module_name_def=module_name.replace('.','_').upper()
	DEF_NAME="__MY_NEW_EXT_"+module_name_def+"_H__"
	DEF_NAME_IDATA="__MY_NEW_EXT_IDATA_"+module_name_def+"_H__"
	module_gen_fname=os.path.join(dirName,module_name)
	fo=open(module_gen_fname,"w")

	fo.write(headers_defs)
	

	module_name_h=module_name.replace('.c','.h')
	if (h_prefix!=None) and(h_prefix!=""):
		inc_str="\""+h_prefix+"/"+module_name_h+"\""
	else:
		inc_str="\""+module_name_h+"\""
	fo.write("\n#include "+inc_str+"\n")
	fo.write(funcs_rels)
	fo.close()
	module_gen_fname=os.path.join(dirName,module_name_h)
	fo=open(module_gen_fname,"w")
	fo.write("\n#ifndef "+DEF_NAME+"\n"+
	"#define "+DEF_NAME+"\n")
	fo.write(headers_defs)
	#fo.write()
	fo.write(funcs_defs)
	fo.write("\n#endif\n")
	fo.close()
	module_idata_h=module_name_h.replace('.h','_idata.h')
	if (h_prefix!=None) and(h_prefix!=""):
		inc_str="\""+h_prefix+"/"+module_name_h+"\""
	else:
		inc_str="\""+module_name_h+"\""
	module_gen_fname=os.path.join(dirName,module_idata_h)
	fo=open(module_gen_fname,"w")
	
	fo.write("#ifndef "+DEF_NAME_IDATA+"\n"+ """
#define """+DEF_NAME_IDATA+"\n" )
	fo.write(jel_struct_native_declares)
	fo.write("""
#endif
""")
	fo.close()
	print "Complete gen "+str(len(funcs))+" prototypes native functions!"
if __name__=="__main__":
	#gen_jni_interface_to_file(r"C:\algoritm\c_src\JVMs\tinyvm_0_2_6\classes\natives_from_muvm.lst")
	gen_jni_interface_to_file(r"C:\algoritm\c_src\JVMs\midpath-0.3rc2\components\core\natives.lst")
	#gen_jni_interface_to_file(r'C:\algoritm\leJosDevSrc\classes\native_mod.rep')
	