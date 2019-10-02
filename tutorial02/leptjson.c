#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')
#define CHECKALLDIGIT(c) \
do {\
	while (1)	\
	{	\
		if (ISDIGIT(*c->json)) \
		{\
			c->json++;\
		}\
		else\
		{\
			break;\
		}\
	}\
	if (*c->json == '\0')\
	{\
		return LEPT_PARSE_OK;\
	}\
}while(0)

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* compareStr, int literalType) {
	EXPECT(c, *compareStr);

	size_t i;

	for (i = 0; compareStr[i + 1]; i++)
	{
		if (c->json[i] != compareStr[i + 1])
		{
			return LEPT_PARSE_INVALID_VALUE;
		}
	}

	c->json = c->json + i;
	v->type = literalType;
	return LEPT_PARSE_OK;
}

static int lept_number_valid(lept_context* c, lept_value* v){
	// 检验负号
	if (*c->json == '-')
		c->json++;

	if (!ISDIGIT(*c->json))
	{
		return LEPT_PARSE_INVALID_VALUE;
	}

	// 整数部分
	if (*c->json == '0')
	{
		c->json++;
	}
	else
	{
		while(1)
		{
			if (ISDIGIT1TO9(*c->json))
			{
				c->json++;
			}
			else
			{
				break;
			}
		}
	}

	// 小数
	if (*c->json == '.')
	{
		// frac
		c->json++;

		if (!ISDIGIT(*c->json))
		{
			return LEPT_PARSE_INVALID_VALUE;
		}

		CHECKALLDIGIT(c);
	}

	// 指数
	if (*c->json == 'e' || *c->json == 'E')
	{
		// exp
		c->json++;
		if (*c->json == '+' || *c->json == '-')
		{
			c->json++;
		}

		CHECKALLDIGIT(c);
	}
	else if(*c->json == '\0')
	{
		// end
		return LEPT_PARSE_OK;
	}
	else
	{
		return LEPT_PARSE_INVALID_VALUE;
	}
}

static int lept_parse_number(lept_context* c, lept_value* v) {
	const char* ptr = c->json;

	int parseResult = lept_number_valid(c, v);
	if (parseResult != LEPT_PARSE_OK)
	{
		return parseResult;
	}

	errno = 0;
    v->n = strtod(ptr, NULL);
	if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't': return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
