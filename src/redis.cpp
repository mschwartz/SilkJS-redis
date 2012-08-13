#include <SilkJS.h>
#include "hiredis/hiredis.h"

static JSVAL connect(JSARGS args) {
    char *host = strdup("127.0.0.1");
    int port = 6379;
    if (args.Length() > 0) {
        free(host);
        String::Utf8Value _host(args[0]->ToString());
        host = strdup(*_host);
    }
    if (args.Length() > 1) {
        port = args[1]->IntegerValue();
    }

    struct timeval timeout = { 1, 500000};  // 1.5 seconds
    redisContext *c = redisConnectWithTimeout(host, port, timeout);
    if (c->err) {
        printf("connect error %s %d - %s\n", host, port, c->errstr);
        Handle<String> s = String::New(c->errstr);
        redisFree(c);
        free(host);
        return ThrowException(String::Concat(String::New("Redis connect error: "), s));
    }
    free(host);
    return External::New(c);
}

static JSVAL close(JSARGS args) {
    redisContext *c = (redisContext *)JSEXTERN(args[0]);
    redisFree(c);
    return Undefined();
}

static JSVAL query(JSARGS args) {
    redisContext *c = (redisContext *)JSEXTERN(args[0]);
    String::Utf8Value _query(args[1]->ToString());
    redisReply *reply = (redisReply *)redisCommand(c, *_query);

    switch (reply->type) {
        case REDIS_REPLY_STATUS:
        case REDIS_REPLY_STRING:
            {
                Handle<String> s = String::New(reply->str, reply->len);
                freeReplyObject(reply);
                return s;
            }
        case REDIS_REPLY_ARRAY:
            {
                Handle<Array> a = Array::New();
                int ndx = 0;
                for (int j=0; j<reply->elements; j++) {
                    redisReply *element = reply->element[j];
                    switch (element->type) {
                        case REDIS_REPLY_STATUS:
                        case REDIS_REPLY_STRING:
                            a->Set(ndx++, String::New(element->str, element->len));
                            break;
                        case REDIS_REPLY_INTEGER:
                            a->Set(ndx++, Number::New(element->integer));
                            break;
                        case REDIS_REPLY_NIL:
                            a->Set(ndx++, Null());
                            break;
                        case REDIS_REPLY_ERROR:
                            {
                                Handle<String>s = String::New(reply->str, reply->len);
                                freeReplyObject(reply);
                                return ThrowException(String::Concat(String::New("Redis query error: "), s));
                            }
                    }
                }
                freeReplyObject(reply);
                return a;
            }
        case REDIS_REPLY_INTEGER:
            {
                Handle<Number>n = Number::New(reply->integer);
                freeReplyObject(reply);
                return n;
            }
        case REDIS_REPLY_NIL:
            freeReplyObject(reply);
            return Null();
        case REDIS_REPLY_ERROR:
            {
                Handle<String>s = String::New(reply->str, reply->len);
                freeReplyObject(reply);
                return ThrowException(String::Concat(String::New("Redis query error: "), s));
            }
    }
}

static bool isspace(char c) {
    return c == ' ' || c == '\t';
}
static JSVAL command(JSARGS args) {
    redisContext *c = (redisContext *)JSEXTERN(args[0]);
    String::Utf8Value _query(args[1]->ToString());
    char *argstr = strdup(*_query);
    char *in = argstr;
    int n = 0;
    while (*in) {
        while (isspace(*in)) in++;
        if (*in) {
            n++;
            if (*in == '"') {
                in++;
                while (*in && *in != '"') in++;
                if (*in == '"') in++;
            }
            else {
                while (*in && !isspace(*in)) in++;
            }
        }
    }
    in = argstr;
    int argc = n;
    char *argv[n];
    n = 0;
    while (*in) {
        while (isspace(*in)) in++;
        if (*in) {
            if (*in == '"') {
                in++;
                argv[n++] = in;
                while (*in && *in != '"') in++;
                if (*in == '"') {
                    *in = '\0';
                    in++;
                }
            }
            else {
                argv[n++] = in;
                while (*in && !isspace(*in)) in++;
                *in++ = '\0';
            }
        }
    }
    // printf("%s\n", *_query);
    // redisReply *reply = (redisReply *)redisCommand(c, *_query);
    redisReply *reply = (redisReply *)redisCommandArgv(c, argc, (const char **)argv, NULL);
    free(argstr);
    return External::New(reply);    
}

static JSVAL freeReply(JSARGS args) {
    redisReply *reply = (redisReply *)JSEXTERN(args[0]);
    freeReplyObject(reply);
    return Undefined();
}

static JSVAL getType(JSARGS args) {
    redisReply *reply = (redisReply *)JSEXTERN(args[0]);
    return Integer::New(reply->type);
}

static JSVAL getInteger(JSARGS args) {
    redisReply *reply = (redisReply *)JSEXTERN(args[0]);
    return Number::New(reply->integer);
}

static JSVAL getString(JSARGS args) {
    redisReply *reply = (redisReply *)JSEXTERN(args[0]);
    return String::New(reply->str, reply->len);
}

static JSVAL getElements(JSARGS args) {
    redisReply *reply = (redisReply *)JSEXTERN(args[0]);
    return Integer::New(reply->elements);
}

static JSVAL getElement(JSARGS args) {
    redisReply *reply = (redisReply *)JSEXTERN(args[0]);
    int ndx = args[1]->IntegerValue();
    return External::New(reply->element[ndx]);
}

extern "C" JSOBJ getExports() {
    JSOBJT o = ObjectTemplate::New();
    
    o->Set(String::New("REPLY_STRING"), Integer::New(REDIS_REPLY_STRING));
    o->Set(String::New("REPLY_ARRAY"), Integer::New(REDIS_REPLY_ARRAY));
    o->Set(String::New("REPLY_INTEGER"), Integer::New(REDIS_REPLY_INTEGER));
    o->Set(String::New("REPLY_NIL"), Integer::New(REDIS_REPLY_NIL));
    o->Set(String::New("REPLY_STATUS"), Integer::New(REDIS_REPLY_STATUS));
    o->Set(String::New("REPLY_ERROR"), Integer::New(REDIS_REPLY_ERROR));

    o->Set(String::New("connect"), FunctionTemplate::New(connect));
    o->Set(String::New("close"), FunctionTemplate::New(close));
    o->Set(String::New("query"), FunctionTemplate::New(query));
    o->Set(String::New("command"), FunctionTemplate::New(command));
    o->Set(String::New("freeReply"), FunctionTemplate::New(freeReply));
    o->Set(String::New("getType"), FunctionTemplate::New(getType));
    o->Set(String::New("getInteger"), FunctionTemplate::New(getInteger));
    o->Set(String::New("getString"), FunctionTemplate::New(getString));
    o->Set(String::New("getElements"), FunctionTemplate::New(getElements));
    o->Set(String::New("getElement"), FunctionTemplate::New(getElement));
    return o->NewInstance();
}





