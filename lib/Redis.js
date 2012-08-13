/*global exports, require */

(function() {
    "use strict";

    var driver = require('redis_module'),
        REDIS_REPLY_STRING = driver.REPLY_STRING,
        REDIS_REPLY_ARRAY = driver.REPLY_ARRAY,
        REDIS_REPLY_INTEGER = driver.REPLY_INTEGER,
        REDIS_REPLY_NIL = driver.REPLY_NIL,
        REDIS_REPLY_STATUS = driver.REPLY_STATUS,
        REDIS_REPLY_ERROR = driver.REPLY_ERROR;


    function processReply(reply) {
        switch (driver.getType(reply)) {
            case REDIS_REPLY_STRING:
                return driver.getString(reply);
            case REDIS_REPLY_ARRAY:
                var result = [],
                    count = driver.getElements(reply);
                for (var i=0; i<count; i++) {
                    result.push(processReply(driver.getElement(reply, i)));
                }
                return result;
            case REDIS_REPLY_INTEGER:
                return driver.getInteger(reply);
            case REDIS_REPLY_NIL:
                return null;
            case REDIS_REPLY_STATUS:
                return driver.getString(reply);
            case REDIS_REPLY_ERROR:
                throw new Error(driver.getString(reply));
        }
    }

    function Redis(host, port) {
        host = host || 'localhost';
        port = port || 6379;
        this.handle = driver.connect(host, port);
    }
    Redis.prototype.extend({
        command: function(query) {
            var reply = driver.command(this.handle, query);
            var result;
            try {
                result = processReply(reply);
                driver.freeReply(reply);
                return result;
            }
            catch (e) {
                driver.freeReply(reply);
                throw e;
            }
        }
    });

    exports.extend({
        Redis: Redis
    });

}());