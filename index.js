/*global exports, require */
(function() {
    "use strict";

    exports.extend({
        driver: require('./lib/redis_module'),
        Redis: require('./lib/Redis').Redis
    });

}());
