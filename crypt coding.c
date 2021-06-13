/*
 * Menu: Decode or Encode text
 * Author: starsystems.net@gmail.com
 * Date: 2021-1-16
 * */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <jni.h>
#include <string.h>
#include <android/log.h>

#define  LOG_TAG    "com.yaokan.sdk.crypt"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

char char_ascii[]={0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46}; //0123456789ABCDEF对应的ascii码值
char *  char_value = "0123456789ABCDEF";//加解密字符串
int     getIndexChar(const char* chars,char c);//获取字符串中的索引
jsize   getSizejstring(JNIEnv *env, jstring jstr);//获取字符串中的长度
char *  jstringToChar(JNIEnv *env, jstring jstr);//把字符串变为char*数组
jstring chartoJstring(JNIEnv* env, const char* pat,int count);//char* 转换为jstring
jstring encodeZip2(JNIEnv* env, jintArray code);//zip2 加密算法
jstring decodeZip2(JNIEnv* env, jstring code); //zip2 解密算法
jstring decodeZip1(JNIEnv* env, jstring code); //zip1 解密算法
jstring simpleDecodeZip(JNIEnv* env, jstring code); //zip 解密算法
jstring simpleEncodeZip(JNIEnv* env, jstring code);//zip2 加密算法

/*
 * Class:     com_yaokan_crypt_NativeCrypt
 * Method:    encode
 * Signature: (ILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yaokan_sdk_wifi_NativeCrypt_encode(JNIEnv *env, jobject obj, jintArray code){
	//只支持1，2加密算法
	int type = 2;
	if(code == NULL){
		return  (*env)->NewStringUTF(env,"不合法的字符串");
	}
	if(type == 2){
		return encodeZip2(env,code);
	}
	return  (*env)->NewStringUTF(env,"aaa");
}

/*
 * Class:     com_yaokan_crypt_NativeCrypt
 * Method:    encode
 * Signature: (ILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yaokan_sdk_wifi_NativeCrypt_simpleEncode(JNIEnv *env, jobject obj, jstring code){
	if(code == NULL){
		return  (*env)->NewStringUTF(env,"不合法的字符串");
	}
	return simpleEncodeZip(env,code);
}

/*
 * Class:     com_yaokan_crypt_NativeCrypt
 * Method:    decode
 * Signature: (ILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_yaokan_sdk_wifi_NativeCrypt_decode(JNIEnv *env, jobject obj, jstring code){
	if(code == NULL){
		return  (*env)->NewStringUTF(env,"不合法的字符串");
	}
	int type = -1;
	char * encode = jstringToChar(env, code);
	type = encode[1] - 48; //根据传入的第二几个字符判断是哪个算法：1：为压缩1算法，2为压缩2算法
//	LOGI("type:%d",type);
	free(encode);
	//如果type不等于1，或者2，则为不支持的算法
	if( type != 1 && type != 2){
		return  (*env)->NewStringUTF(env,"不支持的压缩算法");
	}
	if(type == 2){
		//压缩2 解密
		return decodeZip2(env,code);
	}else {
		//压缩1 解密
		return decodeZip1(env,code);
	}
	return (*env)->NewStringUTF(env,"");
}

/**
 * 获取jstring的长度
 */
jsize  getSizejstring(JNIEnv *env, jstring jstr){
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env,"java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env,"utf-8");
	jmethodID mid = (*env)->GetMethodID(env,clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr= (jbyteArray)(*env)->CallObjectMethod(env,jstr, mid, strencode);
	jsize alen = (*env)->GetArrayLength(env,barr);
	jbyte* ba = (*env)->GetByteArrayElements(env,barr, JNI_FALSE);
	(*env)->ReleaseByteArrayElements(env,barr, ba, 0);
	return alen;
}
//把 jstring 转换为 char*
char * jstringToChar(JNIEnv *env, jstring jstr)
{
	char* rtn = NULL;
	jclass clsstring = (*env)->FindClass(env,"java/lang/String");
	jstring strencode = (*env)->NewStringUTF(env,"utf-8");
	jmethodID mid = (*env)->GetMethodID(env,clsstring, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray barr= (jbyteArray)(*env)->CallObjectMethod(env,jstr, mid, strencode);
	jsize alen = (*env)->GetArrayLength(env,barr);
	jbyte* ba = (*env)->GetByteArrayElements(env,barr, JNI_FALSE);
	if (alen > 0)
	{
		rtn = (char*)malloc(alen + 1);
		memcpy(rtn, ba, alen);
		rtn[alen] = 0;
	}
	(*env)->ReleaseByteArrayElements(env,barr, ba, 0);
    return rtn;
}

// 把 char* 转换 jstring
jstring chartoJstring(JNIEnv* env, const char* pat,int count)
{
	jclass strClass = (*env)->FindClass(env,"java/lang/String");
	jmethodID ctorID = (*env)->GetMethodID(env,strClass, "<init>", "([BIILjava/lang/String;)V");
	jbyteArray bytes = (*env)->NewByteArray(env,strlen(pat));
	(*env)->SetByteArrayRegion(env,bytes, 0, strlen(pat), (jbyte*)pat);
	jstring encoding = (*env)->NewStringUTF(env,"utf-8");
	jstring result = (jstring)(*env)->NewObject(env,strClass, ctorID, bytes,0,count, encoding);
	return result ;
}

/**
 * 压缩2算法加密
 * 根据传入的整形数组，对频率进行frey/1000 在取整数，对后面字符按照>255 用00标识，<255用两位16进制表示
 */
jstring encodeZip2(JNIEnv* env, jintArray code)
{
	jint* arr;
	jint length;
	arr = (*env)->GetIntArrayElements(env,code,NULL);
	length = (*env)->GetArrayLength(env,code);
	char * pat = (char*)malloc(sizeof(char)*1024*2);
	int pos = 0 ;
	for(int i = 0 ; i < length ; i++){
		int data = arr[i];
		//频率除以1000
		if(i == 1){
			data = data/1000;
		}
		char temp ;
		//其它字符大于255，以00表示，后面四位16进制表示值的大小
		if(data>255){
			pat[pos++] = '0';
			pat[pos++] = '0';
			temp = data/256 ;
			pat[pos++] = char_ascii[temp/16];
			pat[pos++] = char_ascii[temp%16];
			temp = data%256 ;
			pat[pos++] = char_ascii[temp/16];
			pat[pos++] = char_ascii[temp%16];
		}else{
			//小于255的用2位16进制表示大小
			pat[pos++] = char_ascii[data/16];
			pat[pos++] = char_ascii[data%16];
		}
	}
    return chartoJstring(env,pat,pos);
}

/**
 * 压缩2算法解密
 */
jstring decodeZip2(JNIEnv* env, jstring code){
	int size = getSizejstring(env,code)/2 ;
	char * encode = jstringToChar(env, code);
	int pos = 0 ;
	char * decode = (char*)malloc(1024*2);
	decode[0] = '\0';
	int decodeNum = 0;
	for(int i = 0 ; i < size ;i++){
		char high = encode[2*i];
		char low = encode[2*i+1];
//		LOGI("resultzip2 start :%d,%c,%c",i,high,low);
		int forward = getIndexChar(char_value,high) ;
		int next = getIndexChar(char_value,low);
		char  s[10] = {0} ;
		//当相邻2位为00时，则后面4位为数值的值
		if(forward == 0 && next == 0){
			i++;
			high = encode[2*i];
			low = encode[2*i+1];
			forward = getIndexChar(char_value,high) ;
			next = getIndexChar(char_value,low);
			int highValue = forward*16+next;
			i++;
			high = encode[2*i];
			low = encode[2*i+1];
			forward = getIndexChar(char_value,high) ;
			next = getIndexChar(char_value,low);
			int lowValue = forward*16+next;
			int value = 0 ;
//			LOGI("resultzip2 0000 :%d,%d",highValue,lowValue);
			value = highValue * 256 + lowValue ;
			sprintf(s, "%d", value);
			strcat(s, ",");
			strcat(decode, s);
			decodeNum = decodeNum + strlen(s);
		}else{
			int value = forward * 16 + next ;
			if(i==1){
				value = value * 1000;
			}
			sprintf(s, "%d", value);
			strcat(s, ",");
			strcat(decode, s);
			decodeNum = decodeNum + strlen(s);
		}
//		LOGI("resultzip2 :%d,%s",i,s);
	}
//	LOGI("resultzip2 :%s",decode);
	jstring result = chartoJstring(env,decode,decodeNum-1);
	free(encode);
	free(decode);
	return result ;
}

jstring decodeZip1(JNIEnv* env, jstring code){
	int size = getSizejstring(env,code)/2 ;
	char * encode = jstringToChar(env, code);
	int pos = 0 ;
	char * decode = (char*)malloc(1024*2);
	decode[0] = '\0';
	int decodeNum = 0;
	int bitIndexCount = 0 ;
	int bit[4] ;
	for(int i = 0 ; i < size ;i++){
		char high = encode[2*i];
		char low = encode[2*i+1];
		int forward = getIndexChar(char_value,high) ;
		int next = getIndexChar(char_value,low);
		char  s[10] = {0} ;
		int value = forward * 16 + next ;
		//第一个表示位
		if(i==0){
			sprintf(s, "%d", value);
			strcat(s, ",");
			strcat(decode, s);
			decodeNum = decodeNum + strlen(s);
			LOGI("result :%d,%s",i,s);
			continue;
		}
		//第二个频率
		if(i==1){
			value = value * 1000;
			sprintf(s, "%d", value);
			strcat(s, ",");
			strcat(decode, s);
			decodeNum = decodeNum + strlen(s);
			LOGI("result :%d,%s",i,s);
			continue;
		}
		//0，1标识位
		if(bitIndexCount<4){
			high = encode[2*i];
			low = encode[2*i+1];
			forward = getIndexChar(char_value,high) ;
			next = getIndexChar(char_value,low);
			value = forward * 16 + next;
			if(value == 0 ){
				i++;
				high = encode[2*i];
				low = encode[2*i+1];
				forward = getIndexChar(char_value,high) ;
				next = getIndexChar(char_value,low);
				int highValue = forward*16+next;
				i++;
				high = encode[2*i];
				low = encode[2*i+1];
				forward = getIndexChar(char_value,high) ;
				next = getIndexChar(char_value,low);
				int lowValue = forward*16+next;
				value = highValue * 256 + lowValue ;
				sprintf(s, "%d", value);
				strcat(s, ",");

			}else{
				sprintf(s, "%d", value);
				strcat(s, ",");
			}
			bit[bitIndexCount] = value ;
//			LOGI("result :%d,%d",i,bit[bitIndexCount]);
			bitIndexCount++;
			continue;
		}
//		LOGI("result2 :%c,%c",high,low);
		high = encode[2*i];
		low = encode[2*i+1];
		//当遇到ff时，为不是逻辑0，1的值，
		if(high == 'f' && low == 'f'){
			i = i + 2 ;
			high = encode[2*i];
			low = encode[2*i+1];
//			LOGI("result3 :%c,%c",high,low);
			forward = getIndexChar(char_value,high) ;
			next = getIndexChar(char_value,low);
			int valueLen = forward * 16 + next ;
			i++;
//			LOGI("result3 :%d",valueLen);
			for(int j = 0 ; j < valueLen ; j++){
				high = encode[2*(i+j)];
				low = encode[2*(i+j)+1];
				LOGI("result3 :%c,%c",high,low);
				forward = getIndexChar(char_value,high) ;
				next = getIndexChar(char_value,low);
				value = forward * 16 + next ;
				if(value == 0){
					j++;
					high = encode[2*(i+j)];
					low = encode[2*(i+j)+1];
					forward = getIndexChar(char_value,high) ;
					next = getIndexChar(char_value,low);
					int highValue = forward*16+next;
					j++ ;
					high = encode[2*(i+j)];
					low = encode[2*(i+j)+1];
					forward = getIndexChar(char_value,high) ;
					next = getIndexChar(char_value,low);
					int lowValue = forward*16+next;
					value = highValue * 256 + lowValue ;
					sprintf(s, "%d", value);
					strcat(s, ",");
					strcat(decode, s);
					decodeNum = decodeNum + strlen(s);
				}else{
					sprintf(s, "%d", value);
					strcat(s, ",");
					strcat(decode, s);
					decodeNum = decodeNum + strlen(s);
				}
			}
			i = i + valueLen - 1 ;
			continue;
		}
//		LOGI("result4 :%c,%c",high,low);
		//逻辑0，1标识的处理方法
		high = encode[2*i];
		low = encode[2*i+1];
		forward = getIndexChar(char_value,high) ;
		next = getIndexChar(char_value,low);
		value = forward * 16 + next;
		unsigned char temp = (unsigned char) value ;
//		LOGI("result4 :%d",temp);
//		LOGI("result4 :%d,%d,%d,%d",bit[0],bit[1],bit[2],bit[3]);
		for(int i = 7 ; i >=0 ;i--){
			int bitvalue = (temp & (1<<i)) ;
//			LOGI("result4 :%d",bitvalue);
			if(bitvalue == 0){
				sprintf(s, "%d", bit[0]);
				strcat(s, ",");
				strcat(decode,s);
				decodeNum = decodeNum + strlen(s);
				sprintf(s, "%d", bit[1]);
				strcat(s, ",");
				strcat(decode,s);
				decodeNum = decodeNum + strlen(s);
			}else{
				sprintf(s, "%d", bit[2]);
				strcat(s, ",");
				strcat(decode,s);
				decodeNum = decodeNum + strlen(s);
				sprintf(s, "%d", bit[3]);
				strcat(s, ",");
				strcat(decode,s);
				decodeNum = decodeNum + strlen(s);
			}

		}
//		LOGI("result :%d,%s",i,s);
	}
//	LOGI("result final :%s",decode);
	jstring result =  chartoJstring(env,decode,decodeNum-1);
	free(encode);
	free(decode);
	return result ;
}

/**
 * 解密算法，还原原始的加密
 */
JNIEXPORT jstring JNICALL Java_com_yaokan_sdk_wifi_NativeCrypt_simpleDecode(JNIEnv * env, jobject obj, jstring code){
	char * encode = jstringToChar(env, code);
	jstring result = NULL;
	int type = encode[1]-48;
//	LOGI("type:%d",type);
	result = simpleDecodeZip(env,code);
	free(encode);
	return result ;
}

/**
 * 解密算法
 */
jstring simpleDecodeZip(JNIEnv* env, jstring code){
	char * encode = jstringToChar(env, code);
	int size = getSizejstring(env,code);
//	LOGI("result simpleDecodeZip1 :%d",size);
	//前面4位不做处理
	int pos = 0 ;
	char * decode = (char*)malloc(1024*2);
	decode[0] = '\0';
	decode[0] = encode[0];
	decode[1] = encode[1];
	decode[2] = encode[2];
	decode[3] = encode[3];
	pos = 4 ;
	int length = (size/2-2);
	//后面没两位代表一个char 值
	for(int i = 0 ; i < length ;i++){
		char forward = encode[2*(i+2)];
		char next = encode[2*(i+2)+1];
		char tempData =  (getIndexChar(char_value,forward)*16 + getIndexChar(char_value,next))^(i%256)  ;
//		LOGI("result simpleDecodeZip1 :%d,%c",i,tempData);
		decode[pos++] = tempData ;
	}
//	LOGI("result pos :%d",pos);
	jstring result =  chartoJstring(env,decode,pos);
	free(encode);
	free(decode);
	return result ;
}

/**
 * 加密算法
 */
jstring simpleEncodeZip(JNIEnv* env, jstring code){
	char * encode = jstringToChar(env, code);
	int size = getSizejstring(env,code);
	char * pat = (char*)malloc(sizeof(char)*1024*2);
	pat[0] ='\0';
	int pos = 0 ;
	//前面4位不做处理
	pat[pos++] = encode[0];
	pat[pos++] = encode[1];
	pat[pos++] = encode[2];
	pat[pos++] = encode[3];
	//后面一个值与索引以后用两位16进制的值表示
	for(int i = 4 ; i < size ; i++){
		int data = encode[i] ^ ( (i-4) % 256);
		pat[pos++] = char_ascii[data/16];
		pat[pos++] = char_ascii[data%16];
	}



	return chartoJstring(env,pat,pos);
}

/**
 * 获取字符串索引
 */
int getIndexChar(const char * chars, const char c){
	if(chars == NULL){
		return -1 ;
	}
	int len = strlen(chars);
	for(int i = 0 ; i < len ;i++){
		if(chars[i] == c){
			return i ;
		}
	}
	return -1 ;
}

