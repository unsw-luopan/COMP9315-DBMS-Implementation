//
//  email.c
//
//
//
//COMP9315 Database Systems Implementation
//Assignment 1
//Group Work
//Group members:
//Pan Luo z5192086
//Zhidong Luo z5181142
//Shuxiang Zou z5187969
//
// We implement a new data type called "EmailAddr". This is an email
//data type which checks whether the email address is valid. We follow
//the instructions given by assignment spec.
//
#include "postgres.h"
#include "fmgr.h"
#include "../include/utils/geo_decls.h"
#include "access/hash.h"

// define struct
typedef struct {
    char _vl_len[4];
    char addr[4];
}EmailAddress;

PG_MODULE_MAGIC;

Datum    email_in(PG_FUNCTION_ARGS);
Datum    email_out(PG_FUNCTION_ARGS);
Datum    e_lt(PG_FUNCTION_ARGS);
Datum    e_le(PG_FUNCTION_ARGS);
Datum    e_eq(PG_FUNCTION_ARGS);
Datum    e_noteq(PG_FUNCTION_ARGS);
Datum    e_gt(PG_FUNCTION_ARGS);
Datum    e_ge(PG_FUNCTION_ARGS);
Datum    e_same_domain(PG_FUNCTION_ARGS);
Datum    e_not_same_domain(PG_FUNCTION_ARGS);
Datum    e_cmp(PG_FUNCTION_ARGS);
Datum    e_hval(PG_FUNCTION_ARGS);
int      good_mail(char *pSrc);

//check whether the input email type is vaild
int good_mail(char *pSrc)
{
    char* pToken = NULL;
    char* pSave = NULL;
    char* plocal = NULL;
    char* plocal_1 = NULL;
    char* c = ".";
    char* pdomain = NULL;
    char* pdomain_1 = NULL;
    char* pDelimiter = "@";
    int count = 0;
    int count_doc=0;
    int i = 0;

    for (i=0;pSrc[i];i++){
        if (pSrc[i] == '@') {
            count++;
        }
        if ( !(isalpha(pSrc[i]) || isdigit(pSrc[i]) ||
               pSrc[i] == '-'   || pSrc[i] == '@'   || pSrc[i] == '.') ) {
            return 0;
        }
        if ((pSrc[i]=='.' && pSrc[i+1]=='.') || (pSrc[i]=='-' && pSrc[i+1]=='-')){
            return 0;
        }
    }
    pToken = strtok_r(pSrc, pDelimiter, &pSave);
    if (strlen(pToken) > 256 || strlen(pSave) > 256){
    return 0;
}
    for (i=0;pSave[i];i++){
        if (pSave[i]=='.'){
            count_doc++;
        }
    }
    if (pToken[strlen(pToken)-1] == '.' || pSave[strlen(pSave)-1] == '.' || count > 1 || count_doc < 1) {
        return 0;
    }
    plocal=strtok_r(pToken, c, &plocal_1);
    while(plocal){
        int a=strlen(plocal);
        if (!(plocal[0]>='a' && plocal[0]<='z')) {
            return 0;
        }
        if (!((plocal[a-1]>='a' && plocal[a-1]<='z')||(plocal[a-1]>='0'&&plocal[a-1]<='9'))) {
            return 0;
        }
        plocal = strtok_r(NULL, c, &plocal_1);
    }
    pdomain=strtok_r(pSave, c, &pdomain_1);
    if (pdomain_1 == NULL) {
        return 0;
    }
    while(pdomain){
        int a=strlen(pdomain);
        if (!(pdomain[0]>='a' && pdomain[0]<='z')) {
            return 0;
        }
        if (!((pdomain[a-1]>='a' && pdomain[a-1]<='z')||(pdomain[a-1]>='0'&&pdomain[a-1]<='9'))) {
            return 0;
        }
        pdomain = strtok_r(NULL, c, &pdomain_1);
    }
    return 1;
}


// the input function when there is an input request in database
PG_FUNCTION_INFO_V1(email_in);

Datum email_in(PG_FUNCTION_ARGS){
    char *str;
    int  i;
    int len;
    EmailAddress *result;
    str = (char *)PG_GETARG_CSTRING(0);
    char newstr[strlen(str)+1];
    //convert to connical form
     for (i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }

    strcpy(newstr,str);
    if ( good_mail(newstr)==0 ) {
        ereport( ERROR, (errcode (ERRCODE_INVALID_TEXT_REPRESENTATION),
                         errmsg ("invalid input syntax for EmailAddress: \"%s\"", str)));
    }
    len=strlen(str)+1;
    result = (EmailAddress *) palloc(VARHDRSZ+len);
    SET_VARSIZE(result,VARHDRSZ+len);
    memcpy(result->addr, str, len);

    PG_RETURN_POINTER(result);
}



// the output function when there is an output request in database
PG_FUNCTION_INFO_V1(email_out);

Datum  email_out(PG_FUNCTION_ARGS) {
    EmailAddress *str = (EmailAddress *) PG_GETARG_POINTER(0);
    int len = VARSIZE_ANY_EXHDR(str);

    char *result;
    result = (char *) palloc(len);
    snprintf(result, len, "%s", str->addr);

    PG_RETURN_CSTRING(result);
}


//the compare function which compare different emails in different ways
static  int e_compare(EmailAddress *a, EmailAddress *b){
    char *patoken  = NULL;
    char *pasave =  NULL;
    char *pbtoken = NULL;
    char *pbsave = NULL;
    char addra[515];
    char addrb[515];
    strcpy(addra,a->addr);
    strcpy(addrb,b->addr);

    patoken = strtok_r(addra,"@",&pasave);
    pbtoken = strtok_r(addrb,"@",&pbsave);

    if (strcmp(pasave, pbsave))
        return strcmp(pasave, pbsave);
    else if (strcmp(patoken, pbtoken))
        return strcmp(patoken, pbtoken);
    else
        return 0;
}


// greater than function
PG_FUNCTION_INFO_V1(e_gt);

Datum e_gt(PG_FUNCTION_ARGS) {
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(e_compare(a, b) > 0);
}

// greater than or equal to function
PG_FUNCTION_INFO_V1(e_ge);

Datum e_ge(PG_FUNCTION_ARGS) {
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(e_compare(a, b) >= 0);
}

// equal to function
PG_FUNCTION_INFO_V1(e_eq);

Datum  e_eq(PG_FUNCTION_ARGS) {
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(e_compare(a, b) == 0);
}

// not equal to funtion
PG_FUNCTION_INFO_V1(e_noteq);

Datum  e_noteq(PG_FUNCTION_ARGS) {
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(e_compare(a, b) != 0);
}

// less than function
PG_FUNCTION_INFO_V1(e_lt);

Datum e_lt(PG_FUNCTION_ARGS) {
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(e_compare(a, b) < 0);
}

// less than or equal to function
PG_FUNCTION_INFO_V1(e_le);

Datum e_le(PG_FUNCTION_ARGS) {
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);

    PG_RETURN_BOOL(e_compare(a, b) <= 0);
}

// check whether the domain of two eamils are the same
PG_FUNCTION_INFO_V1(e_same_domain);

Datum  e_same_domain(PG_FUNCTION_ARGS) {
    char *locala = NULL;
    char *localb = NULL;
    char *domaina = NULL;
    char *domainb = NULL;
    char addra[515];
    char addrb[515];
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);
    strcpy(addra,a->addr);
    strcpy(addrb,b->addr);
    locala = strtok_r(addra,"@",&domaina);
    localb = strtok_r(addrb,"@",&domainb);
    PG_RETURN_BOOL(strcmp(domaina, domainb) == 0);
}

// check whether the domain of two eamils are not the same
PG_FUNCTION_INFO_V1(e_not_same_domain);

Datum  e_not_same_domain(PG_FUNCTION_ARGS) {
    char *locala = NULL;
    char *localb = NULL;
    char *domaina = NULL;
    char *domainb = NULL;
    char addra[515];
    char addrb[515];
    EmailAddress *a  = (EmailAddress *) PG_GETARG_POINTER(0);
    EmailAddress *b = (EmailAddress *) PG_GETARG_POINTER(1);
    strcpy(addra,a->addr);
    strcpy(addrb,b->addr);
    locala = strtok_r(addra,"@",&domaina);
    localb = strtok_r(addrb,"@",&domainb);
    PG_RETURN_BOOL(strcmp(domaina, domainb) != 0);
}

//compare function
PG_FUNCTION_INFO_V1(e_cmp);
Datum e_cmp(PG_FUNCTION_ARGS)
{

 EmailAddress    *a = (EmailAddress *) PG_GETARG_POINTER(0);
 EmailAddress    *b = (EmailAddress *) PG_GETARG_POINTER(1);
 PG_RETURN_INT32(e_compare(a, b));

}

// hash function
PG_FUNCTION_INFO_V1(e_hval);
Datum e_hval(PG_FUNCTION_ARGS)
{
    EmailAddress *address = (EmailAddress *) PG_GETARG_POINTER(0);
    int len = VARSIZE_ANY_EXHDR(address);
    char result[515];
    int i;
    for (i = 0; *(address->addr + i) != '\0'; i++) {
        result[i] = *(address->addr + i);
    }
    result[i] = '\0';
    PG_RETURN_INT32(DatumGetInt32(hash_any((const unsigned char *) result,
                                    len)));
}
