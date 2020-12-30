import string,os,sys
#from os import stat
def b4ToInt(str):
	b1=ord(str[0])
	b2=ord(str[1])
	b3=ord(str[2])
	b4=ord(str[3])
	return (b2<<8)+(b3<<16)+(b4<<24)+b1

def int4data( val):
	i1=val &(0xff)
	i2= (val>>8)&(0xff)
	i3=(val>>16)&(0xff)
	i4=(val>>24)&(0xff)
	b_0=chr(i1)
	b_1=chr(i2)
	b_2=chr(i3)
	b_3=chr(i4)
	return b_0+b_1+b_2+b_3

def intTo4b(i):
	s=''
	a=i & 0xff
	s=s+chr(a)
	a=(i& 0xffff)>>8
	s=s+chr(a)
	a=(i& 0xffffff)>>16
	s=s+chr(a)
	a=(i& 0xffffff)>>24
	s=s+chr(a)
	#s[0]=chr(i%0xff
	#s[0]=chr(i%0xff)
	return s

def extractInt(s,atIndex=0):
	r=''
	i=-1
	#print s
	for c in s:
		i=i+1
		if (i<atIndex):
			continue
		ci=ord(c)
		if (ci>=48)and(ci<=57):
			r+=c
		else:
			break
	return int(r);
def extractLat(s,atIndex=0):
	r=''
	i=-1
	#print s
	for c in s:
		i=i+1
		if (i<atIndex):
			continue
		ci=ord(c)
		if isLat(ci):
			r+=c
		else:
			break
	return r;

def writeInt(fo,i):
	dt=int4data(i)
	fo.write(dt)
def readInt(fi):
	dt=fi.read(4)
	return b4ToInt(dt)

ruUnicode=u'\u0410\u0411\u0412\u0413\u0414\u0415\u0416\u0417\u0418\u0419\u041A\u041B\u041C\u041D\u041E\u041F\u0420\u0421\u0422\u0423\u0424\u0425\u0426\u0427\u0428\u0429\u042A\u042B\u042C\u042D\u042E\u042F\u0430\u0431\u0432\u0433\u0434\u0435\u0436\u0437\u0438\u0439\u043A\u043B\u043C\u043D\u043E\u043F\u0440\u0441\u0442\u0443\u0444\u0445\u0446\u0447\u0448\u0449\u044A\u044B\u044C\u044D\u044E\u044F\u0401\u0451'
rumUnicode=u''
alfabetHex='0123456789ABCDEF'
alfabetHexLow='0123456789abcdef'
def hex2int(s):
	val=0
	for c in s:
		fi=alfabetHex.find(c)
		if (fi<0):
			fi=alfabetHexLow.find(c)
		if (fi<0):
			break
		val=val*16+fi
	return val
translit_table="ABVGDEJZIYKLMNOPRSTUFHQCSS_I`EUYabvgdejziyklmnoprstufhqcss_i`euyEe"
simpleChars="ABCDEFGHIJKLMNOPQRSTUV XYZabcdefghijklmnopqrstuv xyz"
simpleCharsRu=['\xC0','\xC1','\xD7','\xC4','\xC5','\xD4','\xC3','\xD5','\xC8','\xC6','\xCA','\xCB','\xCC','\xCD',
					'\xCE','\xCF','\xD6','\xD0','\xD1','\xD2','\xD3','\xC2','','\xCA\xD1','\xDF','\xC7',
					'\xE0','\xE1','\xF7','\xE4','\xE5','\xF4','\xE3','\xF5','\xE8','\xE6','\xEA','\xEB','\xEC','\xED',
					'\xEE','\xEF','\xF6','\xF0','\xF1','\xF2','\xF3','\xE2','','\xEA\xF1','\xFF','\xE7'
				]#as cp1251
#as cp1251
dopWEncodeArray2='\xFD\xF7\xF6\xF8\xF9\xFF\xE9\xFE\xFB\xB8\xFC\xFA\xDD\xD7\xD6\xD8\xD9\xDF\xC9\xDE\xDB\xA8\xDA\xDC'
dopWEncodeArray1='ectsjaiuyox_ECTSJAIUYOX_'
s=''
u_type=type(u'')
str_type=type(s)
jy=0
if (u_type==str_type):
	jy=1
try:
	import java
	jy=1
except:
	jy=0
#print 'jy:',jy,u_type,str_type
def rum2_low(s):
	 r=''
	 for i in s:
		 ci=ord(i)
		 if (ci==140):r=r+chr(141)
		 elif (ci==128):r=r+chr(129)
		 elif (ci==131):r=r+chr(138)
		 elif (ci==144):r=r+chr(154)
		 elif (ci==142):r=r+chr(143)
		 elif (ci<=127):r=r+i.lower()
		 elif (1):
			 r=r+" "
	 return r

#is lat function
def isHiLat(code):	
	return ((code>=65)and(code<=90))
def isLoLat(code):	
	return ((code>=97)and(code<123))
def isNum(code):
	return (code>47)and(code<58)
def isIntStr(s):
	ret=0
	for c in s:
		ci=ord(c)
		if isNum(ci):
			ret=1
		else:
			return 0
	return ret
def isHexStr(s):
	ret=0
	for c in s:
		ci=ord(c)
		if isNum(ci)or(alfabetHex.find(c)>=0)or (alfabetHexLow.find(c)>=0):
			ret=1
		else:
			return 0
	return ret 

def isLat(code):
	return ((code>=65)and(code<=90))or((code>=97)and(code<123));

def ruChar(code):
	#code=ord(c)
	#print code
	ret=-1
	if (code>191):
		ret=code-192;
	if ((code==168)):
		ret=64
	if ((code==184)):
		ret=65
	if (ret>=0):
		return ruUnicode[ret]
	else:
		if code<128:
			return unichr(code)
	if (code==128): return u'\u00c2'
	if (code==129): return u'\u00e2'
	if (code==131): return u'\u00ce'
	if (code==138): return u'\u00ee'
	if (code==140): return u'\u0102'
	if (code==141): return u'\u0103'
	if (code==142): return u'\u015E'
	if (code==143): return u'\u015f'
	if (code==144): return u'\u0162'
	if (code==154): return u'\u0163'
	return u' '

def unicode2(st):
	ret=u''
	le=len(st)
	i=0
	while i<le:
		c=st[i]
		ci=ord(c)
		if (ci>255):
			ret=ret+unichr(ci)
		else:
			ret=ret+ruChar(ci)
		i=i+1
	return ret


def cp1251ord2uni_ord(ci):
	if (ci<128):
		return ci
	if (ci>=192)and(ci<256):
		return ci+1040-192
	if (ci==168):
		return 1025
	if (ci==184):
		return 1105
	return ci
	
def uni_ord2str_ord(ci):
	ix=-1#ci
	if (ci>255):
		if (ci>=1040)and(ci<1104):
			ix=ci-1040+192
		elif (ci==1025):
			ix=168
		elif (ci==1105):
			ix=184
		ix=ix % 256
	#rumin
	if (ci==0x00c2): return 128
	if (ci==0x00e2): return 129
	if (ci==0x00ce): return 131
	if (ci==0x00ee): return 138
	if (ci==0x0102): return 140
	if (ci==0x0103): return 141
	if (ci==0x015E): return 142
	if (ci==0x015f): return 143
	if (ci==0x0162): return 144
	if (ci==0x0163): return 154
	if (ci==0x2013):# '-' unicode
		return 45
	if (ci==0x00AB):#unicode start '"'
		return 34
	if (ci==0x00BB):#unicode end '"'
		return 34
	if (ci==0x2019):#unicode  "'"
		return 39
	if (ci<=127):
			return ci
	if (ix<0):
		return 1
	return ix

def uni2str(ustr):
	s=''
	for c in ustr:
		ci=ord(c)
		ix=uni_ord2str_ord(ci)		
		s=s+chr(ix)
	return s		   

def cp1251_upper(s):
	r=''	
	for c in s:
		#print 'len_c:',len(c),' c:',c
		ci=ord(c)
		if (ci<128):
			r=r+c.upper()
		else:	   
			if (ci>223):
				r=r+chr(ci-32)
			elif (ci==184):
				r=r+'\xa8'
			else:
				r=r+c
	return r
def cp1251_upper_ci(ci):
	if ((ci<128)and(ci>64)):
		return (ci & 223);
	else:
		if (ci>223):
			return ci-32;
		else:
			if (ci==184):
				return 0xa8;
			else:
				if ci==0x8d: return 0x8c;#ae
				if ci==0x8a: return 0x83;#ii
				if ci==0x81: return 0x80;#ai
				if ci==0x9a: return 0x90;#ts
				if ci==0x8f: return 0x8e;#s,i
				return ci;
	return ci;
def is_upper_string(s):
	for c in s:
		ci=ord(c)
		ci2=cp1251_upper_ci(ci)
		if (ci!=ci2):return 0
	return 1
def cp1251_upper_1_letter(s):
	j,r=-1,''
	for c in s:
		j=j+1
		if j==0:c=cp1251_upper(c)
		r=r+c
	return r
def find_fragment(s,fragment,case_sens,pos=0):
	not_case_sens=not case_sens
	le=len(fragment)
	j=-1
	for c in s:
		j=j+1
		c2=fragment[pos]
		if not_case_sens:
			c2=cp1251_upper(c2)
			c=cp1251_upper(c)
		if (c2==c):
			pos=pos+1
			if (pos>=le):
				return [j,pos]
		else: pos=0
	#if (pos>0):
	#	return (-1,pos)
	return [-1,pos]   
	
def enc_(c):
	ci=ord(c)
	h=(ci/16)%16
	l=ci % 16
	p='\\x'+alfabetHex[h]+alfabetHex[l]
	return p
def enc_u(c):
	ci=ord(c)
	ci1=ci % 256
	ci2=ci / 256
	h=(ci2/16)%16
	l=ci2 % 16
	p='\\u'+alfabetHex[h]+alfabetHex[l]
	h=(ci1/16)%16
	l=ci1 % 16
	p=p+alfabetHex[h]+alfabetHex[l]
	return p
def str_vis(st):
	s=''
	for c in st:
		ci=ord(c)
		if ci>127:
			p=enc_(c)
			s=s+p
		else:
			s=s+c
	return s
def vis2(st):
	s=''
	for c in st:
		#ci=ord(c)
		p=enc_(c)
		s=s+p
	return s
def str_vis_uni(st):
	s=u''
	for c in st:
		ci=ord(c)
		if ci>127:
			p=enc_u(c)
			s=s+p
		else:
			s=s+unicode2(c)
	return s
def vis(st):
	return str_vis(st)
def unicode3(st):
	try:		
		return unicode(st,'UTF8')
	except:
		return u''
def translit_char(ci):
	r=0
	ix=ci
	if (ci>255):
		if (ci>=1040)and(ci<1104):
			ix=ci-1040
			ix=ord(translit_table[ix])
		elif (ci==1025):
			ix=ord('E')
		elif (ci==1105):
			ix=ord('e')
	return ix
			
def translit_uni(su):
	s=u''
	for c in su:
		ci2=translit_char(ord(c))
		s=s+unichr(ci2)
	return s
def rtrim(s):
	s2=''
	i=len(s) -1
	while (i>=0):
		c=s[i]
		ci=ord(c)
		if (ci==32)or(ci<16):
			if len(s2)>0:
				s2=c+s2
		else:
			s2=c+s2
		i=i-1
	return s2
def trim(s):
	s1=trim_left(s)
	s2=trim_right(s1)
	return s2
def trim_old2(s):
	r,oldC,fprobel='',None,1
	for c in s:
		if ((c==' ')or(ord(c)<16)):isprob=1
		else:
			isprob=0
			fprobel=0
		if (c==oldC)and isprob:
			oldC=c
			continue
		oldC=c
		if fprobel and isprob:continue 
		r=r+c
	return r
def trim_left(s):
	r,oldC,fprobel='',None,1
	foundActiveChar=0
	for c in s:
		ci=ord(c)
		if foundActiveChar:
			r+=c
		else:
			if ((ci==32)or((ci<16)and(ci>=0))):
				pass
			else:
				foundActiveChar=1
				r+=c
	return r
def trim_right(s):
	r,oldC,fprobel='',None,1
	foundActiveChar=0
	i=len(s)-1
	if (i<0):
		return''
	#for c in s:
	while i>=0:
		c=s[i]
		if foundActiveChar:
			r=c+r
		else:
			if ((c==' ')or(ord(c)<16)):
				pass
			else:
				foundActiveChar=1
				r=c+r
		i-=1
	return r
	
def trim_str(s):#as trim
	r,oldC,fprobel='',None,1
	start_ix,end_ix=-1,-1
	j=-1
	for c in s:
		j=j+1
		ci=ord(c)
		if ((ci==32)or((ci<16)and(ci>=0))):isprob=1
		else: isprob=0
		#print 'c:',c,' isprob:',isprob
		if (start_ix<0):
			if not isprob:
				start_ix=j#naiden pervii symvol
				end_ix=j
		else:
			if not isprob:end_ix=j#naiden poslednii
	#print 'start_ix:',start_ix,' end_ix',end_ix
	return s[start_ix:end_ix+1]
def trim_str_old_last(s):#as trim
	r,oldC,fprobel='',None,1
	start_ix,end_ix=-1,-1
	for c in s:
		ci=ord(c)
		if ((c==' ')or((ci<16)and(ci>=0))):isprob=1
		else:
			isprob=0
			fprobel=0
		if (c==oldC)and isprob:
			oldC=c
			continue
		oldC=c
		if fprobel and isprob:continue
		if ((ci<16)and(ci>=0)):c=' '
		r=r+c
	return r
def delLastSlash(s):
	fi1=s.rfind('\\')
	fi2=s.rfind('/')
	if (fi2>fi1):
		fi1=fi2
	le=len(s)
	if (fi1>=0) and ((fi1+1)==le):
		return s
	return s[0:fi1]
	
def trim_str_old(s):
	s2=s.replace('\r','')
	s2=s2.replace('\n','')
	find_space=1
	ret=''
	for i in xrange(len(s2)):
		c=s2[i]
		if (c!=' '):
			find_space=0
		if (not find_space):
				ret=ret+c
	ret2=''
	find_space=1
	i=len(ret)-1
	while i>-1:
		c=ret[i]
		if (c!=' '):
			find_space=0
		if (not find_space):
				ret2=c+ret2
		i=i-1
	return ret2

def uni2html_code(su):
	i=0
	s=''
	for c in su:
		ci=ord(c)
		if (ci>127):
			s=s+"&#"+str(ci)+";"
		else:
			s=s+chr(ci)
	return s
def uni2html_code2(su):
	i=0
	s=''
	for c in su:
		#ci=ord(c)
		ci=cp1251ord2uni_ord(ord(c))
		if (ci>127):
			s=s+"&#"+str(ci)+";"
		else:
			s=s+chr(ci)
	return s

def upcase_rum1(ci):
	if (ci==128):return 129#A^->a^
	if (ci==131):return 138#I^->i^
	if (ci==140): return 141#A'->a'
	if (ci==142): return 143#s,
	if (ci==144): return 154#t,
	if (ci>191)and(ci<224):ci=ci+32
	return ci


def low_case_rum1(s):
	s2=''
	for c in s:
		ci=ord(c)
		if (ci<128):
			s2=s2+c.lower()
		else:s2=s2+chr(upcase_rum1(ci))
	return s2


def save_add(li,it):
	for i in li:
		if i==it:return 0
	li.append(it)
	return 1

def add_slash(s,lastChar='/'):
	hi=len(s)-1
	if hi<0: return s#lastChar
	if (s[hi]!=lastChar):
		s=s+lastChar
	return s
def isSpace(c):
	ci=ord(c)
	if (ci==32)or(ci<16):
		return 1
	return 0
def isExt(filename,ext):
	ext=ext.upper()
	ret=filename.upper().endswith(ext)
	return ret
def hasExt(filename,ext):
	return isExt(filename,ext)
def isFormatByExt(fname,exts):
	i=0
	for e in exts:
		ext='.'+e
		if (isExt(fname,ext)):
			return i
		i=i+1
	return -1

def load_option(fname):
	fi=open(fname,'rt')
	#fsz=os.stat(fname)[4]
	data=fi.readlines()
	fi.close()
	return data

def get_opt_param(data,name):
	for s in data:
		ix1=s.find(name)
		if ix1>0:
			s2=s[ix1+len(name):]
			s3=trim(s2)
			return s3
	return None

def chfile(li):
	r=''
	for i in li:
		try:
			fi=open(i,'rt')
			fi.close()
			r=i
			break
		except:
			a=0
	return r
def chdir(li):
	r=''
	for i in li:
		if os.path.exists(i):
			return i
	return r


def path_rz():
	return os.path.sep 
	rz='\\'#Windows
	try:
		b=os.path.isdir('/lib/')
		if b:
		#if (b!=None):
			rz='/'
	except:
		i=0
		rz='\\'
	return rz

def jy2u(s):
	global u_type
	if type(s)==u_type:return s
	if (jy):
		r=u''
		for c in s:
			ci=ord(c)
			lo= ci % 256
			hi=ci /256
			s=chr(hi)+chr(lo) 
			u=unicode(s,'utf16')
			r=r+u
			#r=r+uchr(ci)
		return r
	return unicode(s,'UTF8')		  
	
def join(path,name):
	rz='\\'
	tp_p=0
	tp_n=0
	if type(path)==type('u'):tp_p=1;rz=u'\\'
	if type(name)==type('u'):tp_n=1
	if tp_p:
		if path.find(u'/'):
			rz=u'/'
	else:
		if path.find('/'):
			rz='/'
	if(len(path)>0):
		ch=path[len(path)-1]
	else: return name
	if (ch!=rz):
		path=path+rz
	if (tp_p!=tp_n):
		if (tp_p):
			name=unicode(name,'UTF8')
			return path+name
		else:
			path=unicode(path,'UTF8')
			return path+name 
	return path+name
def addSlash_u(name,rz='\\'):
	global u_type
	name=jy2u(name)
	rz=jy2u(rz)
	ch=u' '
	if(len(name)>0):
		ch=path[len(name)-1]
	if (ch!=rz):
		return name+rz
	return name  
def join_s(path,name,rz='\\'):
	global u_type
	if type(path)==u_type:
		path=path.encode('UTF8')#tp_p=1;rz=u'\\'
	if type(name)==u_type:
		name=name.encode('UTF8')#tp_n=1
	if type(rz)!=u_type:
		rz=rz.encode('UTF8')#tp_n=1
	ch=' '
	if(len(path)>0):
		ch=path[len(path)-1]
	if(len(name)>0):
		ch2=name[0]
	else: return name
	if (ch!=rz)and(ch2!=rz):
		path=path+rz
	return path+name	
def join_u(path,name,rz='\\'):
	global u_type
	path=jy2u(path)
	name=jy2u(name)
	rz=jy2u(rz)
	ch=u' '
	if(len(path)>0):
		ch=path[len(path)-1]
	if(len(name)>0):
		ch2=name[0]
	else: return name
	if (ch!=rz)and(ch2!=rz):
		path=path+rz
	return path+name 
	
def load_data_as_str(ndir,fname=''):
	fi=open(ndir+fname,"rb")
	data=fi.read()
	fi.close()
	return data
def load_data_as_lines(ndir,fname=''):
	fi=open(ndir+fname,"rb")
	data=fi.readlines()
	fi.close()
	return data
def save_data_as_str(ndir,fname,data):
	#fname
	fi=open(ndir+fname,"wb")
	fi.write(data)
	fi.close()
def save_data_as_lines(ndir,fname,data):
	fi=open(ndir+fname,"wb")
	for s in data:
		fi.write(s)
	fi.close()


def delete_tags(s,beginChar='<',endChar='>'):
	findTag=0;
	r=''
	for c in s:
		if (findTag):
			if c==endChar:
				findTag=0
				continue
		else:
			if (c==beginChar):
				findTag=1
				continue
			r=r+c
	return r
translit_table1=('A','B','V','G','D','E','J','Z','I','II','K','L','M','N','O',
'P','R','S','T','U','F','H','TS','C','SH','SHI','-','_I','_','EE','IU','IA' ,
'a','b','v','g','d','e','j','z','i','ii','k','l','m','n','o',
'p','r','s','t','u','f','h','ts','c','sh','shi','-','_i','_','ee','iu','ia' ,'IO','io','?','?','?')

def translit_rus_uchar(su):
	fi=ruUnicode.find(su)
	if (fi>=0):
		return translit_table1[fi]
	ci=ord(su)
	return chr(ci % 256)
def translit_rus_u(su):
	r=''
	for cu in su:
		r=r+translit_rus_uchar(cu)
	return r
def translit_rus_cp1251(s):
	su=unicode2(s)
	return translit_uni(su)

def only_nums(s):
	r=''
	for c in s:
		ci=ord(c)
		if (ci>=48)and(ci<=57):
			r=r+c
	return r
def win2dos(s):#cp1251->cp866
	su=unicode2(s)
	return su.encode("CP866")
	
def sootv_ru(ci):
	if (ci==65):return 192
	if (ci==69):return 197
	return ci
def extract_dir(fname):
	fi=fname.rfind('\\')
	if (fi<0): fi=fname.rfind('/')
	if (fi<0):return ''
	return fname[:fi+1]
def extract_fname(fname):
	fi=fname.rfind('\\')
	if (fi<0): fi=fname.rfind('/')
	if (fi<0):return fname
	return fname[fi+1:]
def change_ext(fname,ext):
	fi=fname.rfind('.')
	if (fi<0):return fname+ext
	return fname[:fi]+ext
	
def listdir_by_exts(ndir,exts):
	list=os.listdir(ndir)
	list2=[]
	for it in list:
		sl=it.lower()
		file_ok=0
		for s in exts:
			if (sl.endswith(s)):
				file_ok=1
				break
		if (file_ok):
			list2.append(it)
	return list2
#added from atlas_bat
def cp1251_to_translit(s):
	uni=unicode2(s)
	uni_lat=translit_uni(uni)
	return uni2str(uni_lat)
#for gavaa_scan
def rus_visible_patch_name(s):
	find_num=0
	j=-1
	r=""
	while(j<(len(s)-1)):
		j=j+1
		c=s[j]
		ci=ord(c)
		if (ci==0xc7):
			ci=48+3
			c="3"
			#if (ci==32)or((ci>=48)and(ci<=57)):
			#r=r+c+s[j+1:]			
			#return r
		if (ci==0xf1):c="c"
		if (ci==0xd1):
			c="C"
		if (ci==0xc7)or ci==0xE7:
			c="3"
		if (ci==0xd0):
			c="P"
		c2=cp1251_to_translit(c)
		r=r+c2
	return r
def delete_tags_by_char(s,c_begin,c_end):
	r=''
	find_tag=0
	for c in s:
		if (c==c_begin):
			find_tag=1
			continue
		elif c==c_end:
			find_tag=0
			continue
		if not(find_tag):r=r+c
	return r

def sort_by_string(it1,it2):
	n1=it1
	n2=it2
	j=-1
	equ=0
	for c in n1:
		j=j+1
		if (j>=len(n2)):
			return -1
		c2=n2[j]
		ci1=ord(c)
		ci2=ord(c2)
		if (ci2>ci1):
			return 1
		if (ci2<ci1):
			return -1
	return 0
def compare_by_string(it1,it2):
	sort_by_string(it1,it2)
get_str_for_sort=None
def get_first_item_str(it):
	return it[0]
def set_get_str_for_sort(metod):
	global get_str_for_sort
	get_str_for_sort=metod
def sort_by_string_items(it1,it2):
	global get_str_for_sort
	n1=get_str_for_sort(it2)
	n2=get_str_for_sort(it1)
	j=-1
	equ=0
	for c in n1:
		j=j+1
		if (j>=len(n2)):
			return -1
		c2=n2[j]
		ci1=ord(c)
		ci2=ord(c2)
		if (ci2>ci1):
			return 1
		if (ci2<ci1):
			return -1
	return 0
def find_in_list(list,s):
	j=-1
	for s2 in list:
		j=j+1
		if s2==s:return j
	return -1
def find_in_list_partial(list,s):
	j=-1
	#print s,list
	for s2 in list:
		j=j+1
		if s.find(s2)>=0:
			return j
	return -1
def find_seq_in_str(s,list):
	for s2 in list:
		fi=s.find(s2)
		if (fi>=0): return fi
	return -1
def help_get_fname(ix,help_files):
	if (ix>=0)and(ix<len(help_files)):
		return help_files[ix]
	return '' 
def help_get_fnames(help_dir,ext=['.txt']):
	#global help_files
	#help_dir=ndir+'help'+path_rz()
	try:
		help_files=listdir_by_exts(help_dir,ext)
	except:
		print sys.exc_info()
		help_files=[]
	#print help_files
	#if (ix>=0)and (ix<len(help_files)):
	return help_files 
	#return None
def help_get_data(ix,help_dir,help_files):
	#global help_files
	s=load_data_as_str(help_dir,help_files[ix])
	return s
def are_num(s):
	for c in s:
		ci=ord(c)
		if (ci>=48)and(ci<=57):return 1
	return 0
def create_encoding(ndir,src_enc,dst_enc,use_translit=0):
	fname=src_enc+'.'+dst_enc
	if (use_translit):fname="t_"+fname
	print fname
	ar=''
	ers=0
	not_found=[]
	for i in xrange(256):
		c=chr(i)
		dstc=c
		#print 'i:',i
		#cu=c.encode(dst_enc)
		#dstc=cu.decode(dst_enc)
		try:
			cu=unicode(c,src_enc)
			if (use_translit):
				cu=translit_uni(cu)
			dstc=cu.encode(dst_enc)
		except:
			#print 'not found:',i
			not_found.append(i)
			#print sys.exc_info()
			dstc=c
			ers=ers+1
		ar=ar+dstc
		#print 'i:',i
	print 'size ar:',len(ar),' Errors:',ers,' not_found:',not_found
	fo=open(ndir+fname,"wb")
	fo.write(ar)
	fo.close()
	
my_shared_dir=''

def get_shared_dir(dname,prefixDir=None):
	global my_shared_dir
	if (prefixDir!=None):
		ret=prefixDir+os.sep+dname+os.sep
		return ret
	dirs=[]
	s1=extract_dir(sys.argv[0])
	s2=get_shared_dir(dname,s1)
	s3=get_shared_dir(dname,os.getcwd())
	s4=get_shared_dir(dname,"/usr/local/share")
	s5=get_shared_dir(dname,"/usr/share")
	s6=get_shared_dir(dname,"/usr/local/share")
	s7=get_shared_dir(dname,"c:\\Files")
	s8=get_shared_dir(dname,"E:\\Files")
	s12=get_shared_dir(dname,"E:\\Files\\shared")
	s9=get_shared_dir(dname,"/c/files")
	s10=get_shared_dir(dname,"Z:\\c\\files")
	s11=get_shared_dir(dname,my_shared_dir)
	dirs=[s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12]
	ret=chdir(dirs)
	return ret
	
def addLoCodeTable(table):
	table2=[]
	le=len(table)
	i=0
	while (i<le):
		it=table[i];
		newIt=[it[0].lower(),it[1].lower()];
		table.append(newIt);
		i=i+1;
	return table
#prevdi Cyrillic encode-decode functions
def  encodeLatChar(c,code_table):
	i=0;
	le=len(code_table)
	while(i<le):
		it=code_table[i];
		#alert(" c:"+c+" it"+it[0]+":"+it[1]);
		if (it[0]==c):
			return it[1];
		if (it[1]==c):
			return it[0];
		i+=1;
	return c;
def encodelat(src,code_table):
	dst='';
	i=0;
	le=len(src)
	while(i<le):
		c=src[i];
		ci=ord(c);
		if (isLat(ci)):
			dst+=encodeLatChar(c,code_table);
		else:
			dst+=c;
		i+=1;
	return dst;	
def ru2lat(src):
	dst='';
	le=len(src);
	i=0;
	while(i<le):
		c=src[i];
		consumed=0;
		j=0;
		simpleCharsRuLen=len(simpleCharsRu);
		while(j<simpleCharsRuLen):
			if (c==simpleCharsRu[j]):
				dst+=simpleChars[j];
				consumed=1;
				break;
			j+=1;
		if (consumed==0):
			dopIx=dopWEncodeArray2.find(c);
			if (dopIx>=0):
				dst+='w'+dopWEncodeArray1[dopIx];
				consumed=1;
		if (consumed==0):
			dst+=c;
		i+=1
	return dst;
def eng2prevdoCirillicEncode(src):
	dst='';
	#for (var i=0;i<src.length;i++)
	i=0;
	srcLen=len(src)
	while (i<srcLen):
		c=src[i];
		code=ord(c);
		if ((code>=65)and(code<90)):
			if (code==87):#W
				j=i+1;
				if (j<srcLen):
					next1=src[j];
					dopIx=dopWEncodeArray1.find(next1);
					if (dopIx>=0):
						dst+=dopWEncodeArray2[dopIx];
				i=j;
			else:
				sc=simpleCharsRu[code-65];
				dst+=sc;
				
		else:
			if((code>=97)and(code<=122)):
				if (code==119):#W
					j=i+1;
					if (j<srcLen):
						next1=src[j];
						dopIx=dopWEncodeArray1.find(next1);
						if (dopIx>=0):
							dst+=dopWEncodeArray2[dopIx];
					
						i=j;	
				
				else:
					sc=simpleCharsRu[code-97+26];
					dst+=sc;	
				
			else:#default
				dst+=c;
		i+=1;
	return dst;
def ruUnicode_find(cu):
	ix=ord(cu)
	if (ix>=1040)and(ix<=1103):
		return ix-1040
	elif (ix==1025):
		return 64
	elif (ix==1105):
		return 65
	return -1
def uchar_to866(cu):
	#ix=ruUnicode.find(cu);
	ix=ruUnicode_find(cu)
	if (ix<0):
		return chr(ord(cu) % 256);
	if (ix>=0)and (ix<48):
		return chr(ix+0x80)
	elif (ix>=48)and(ix<=65):
		return chr(ix+176)

def c866_to_index(c):
	ix=ord(c);
	#if char beetwen 0x80(128)to 175
	if ((ix>=128)and(ix<176)):
		return ix-128;
	if ((ix>=224)and(ix<=241)):
		return ix-176;
	return -1
	
def index_to_uni(ix,origCode):
	if (ix<0)or (ix>65):
		return unichr(origCode)
	if (ix>=0)and(ix<=63):
		return unichr(1040+ix);
	if (ix==64):
		return unichr(1025);
	if (ix==65):
		return unichr(1105)
def c866_to_uni(c):
	ix=ord(c);
	#if char beetwen 0x80(128)to 175
	if ((ix>=128)and(ix<176)):
		return index_to_uni(ix-128,ix);
	if ((ix>=224)and(ix<=241)):
		return index_to_uni(ix-176,ix);
	return index_to_uni(-1,ix)
		
def splitBySpace(s):
	li=[]
	s2=''
	for c in s:
		ci=ord(c)
		if (ci==32)or(ci<16):
			if len(s2)>0:
				li.append(s2)
				s2=''
		else:
			s2=s2+c
	if len(s2)>0:
		li.append(s2)
		s2=''
	return li	

	

if (__name__=="__main__"):
	s=trim_str('1   ')
	s2=delLastSlash("C:\\P\\d\\")
	print s2
	print 's:',s,' le:',len(s)
	"""create_encoding('c:\\musor\\','koi8-r',"cp1251")
	create_encoding('c:\\musor\\','cp866',"cp1251")
	create_encoding('c:\\musor\\','cp1251',"cp1251")
	create_encoding('c:\\musor\\','cp1251',"cp1251",1)
	create_encoding('c:\\musor\\','koi8-r',"cp1251",1)
	create_encoding('c:\\musor\\','cp866',"cp1251",1)"""
	fname='/root/sample1.txt'
	fname="c:\\root\\sample1.txt"
	#print extract_fname(fname)
	s=' String label: label for this menu item'
	s2=trim(s)
	print 's2(trim(s)):'+s2
	s2='[]|( '
	for cu in ruUnicode:
		ru_char_str=cu.encode('UTF-8')
		s2+= '('
		for c in ru_char_str:
			s2+= r'\x%x' % ord(c)
		s2+=')|'
	s2+="){1,}"
	print "ru_utf_regex:",s2
	ru_utf8=ruUnicode.encode('UTF-8')
	
	s=ru2lat(ruUnicode.encode('cp1251'))
	s2=eng2prevdoCirillicEncode(s)
	print s;
	print unicode(s2,'cp1251').encode('utf-8')
	old=0
	j=0
	for cu in ruUnicode:
		cOut=cu.encode('utf-8');
		c866=cu.encode('866');
		code=ord(c866)
		c866_2=uchar_to866(cu);
		#cOut2=c866_2.decode('866').encode('utf-8')
		cOut2=c866_to_uni(c866_2).encode('utf-8')
		print '%i' % ord(cu)+cOut +':%x' % code +':%i'% (code-old) +' j:%i'% j+' :'+cOut2+' index:%i' %c866_to_index(c866) 
		j=j+1
		old=code
def fileExists(path):
	try:
		st=os.stat(path)
	except:
		return 0
	return 1
	
