// https://github.com/mitchellsimoens/redis-session

var redis = require('redis');
var crypto = require('crypto');

function randomValueBase64(len) {
  return crypto.randomBytes(Math.ceil(len * 3 / 4))
    .toString('base64')
    .slice(0, len)
    .replace(/\+/g, '0')
    .replace(/\//g, '0');
}

module.exports = function(config) {
  var mod = this;

  mod.objMerge = function(obj1, obj2) {
    if (obj2) {
      var key;
      var value;

      for (key in obj2) {
        if (obj2.hasOwnProperty(key)) {
          value = obj2[key];

          try {
            // Property in destination object set; update its value.
            if (value.constructor === Object) {
              obj1[key] = mod.objMerge(obj1[key], value);
            } else {
              obj1[key] = value;
            }
          } catch (e) {
            // Property in destination object not set; create it and set its value.
            obj1[key] = value;
          }
        }
      }
    }

    return obj1;
  };

  /**
   * CONFIG SECTION
   * You shouldn't change these but pass in when you create this module:
   *
   *    var session = require('redis-session')({
   *        ttl   : 300000 * 5,
   *        debug : true
   *    });
   */
  config = mod.config = mod.objMerge({
    /**
     * The time to live for the session in milliseconds
     */
    ttl: 900000, // 15min
    /**
     * The number of characters to create the session ID.
     */
    sidLength: 64,
    /**
     * If persist is false, it will expire after the ttl config.
     * If persist is true, it will never expire and ttl config will be ignored.
     */
    persist: false,
    /**
     * REDIS connection information
     */
    connection: {
      port: '6379',
      host: '127.0.0.1',
    },
  }, config);

  mod.objMerge(mod, {
    // @private
    _connect: function() {
      var options = config;
      var client = mod.client;

      if (client) {
        return client;
      }

      client = mod.client = new redis.createClient(options.port, options.host, options);

      if (options.pass) {
        client.auth(options.pass, function(err) {
          if (err) {
            throw err;
          }
        });
      }

      if (options.db) {
        client.select(options.db);

        client.on('connect', function() {
          client.select(options.db);
        });
      }

      return client;
    },

    // @private
    _createSid: function(req, data, callback) {
      var client = mod._connect();
      var sid = randomValueBase64(config.sidLength);

      client.exists(sid, function(err, exists) {
        if (err && process.env.NODE_ENV === 'development') {
          logger.error('----_createSid ERROR');
          logger.error(err);
        }

        if (exists === 1) {
          mod._createSid(req, data, callback);
        } else {
          mod.createSet.call(mod, req, sid, data, callback);
        }
      });
    },

    create: function(req, data, callback) {
      data = data || {};

      var sid = data.sid;

      if (sid) {
        //session active?
        client.exists(sid, function(err, exists) {
          if (err && process.env.NODE_ENV === 'development') {
            logger.error('----_createSid ERROR');
            logger.error(err);
          }

          if (exists === 1) {
            mod._createSid(req, data, callback);
          } else {
            mod.createSet.call(mod, req, sid, data, callback);
          }
        });
      } else {
        mod._createSid.call(mod, req, data, callback);
      }
    },

    createSet: function(req, sid, data, callback) {
      var ttl = config.ttl;
      var persist = config.persist;
      var client = mod._connect();
      var info = JSON.stringify(data);
      var setCallback = function(err, status) {
        if (!err) {
          data.sid = sid;
          req.sessionData = data;
        } else if (process.env.NODE_ENV === 'development') {
          logger.error('----createSet ERROR');
          logger.error(err);
        }

        callback && callback.call(mod, sid, err, status);
      };

      if (persist) {
        client.set(sid, info, setCallback);
      } else {
        client.setex(sid, ttl, info, setCallback);
      }
    },

    get: function(sid, req, callback) {
      var client = mod._connect();

      if (!sid) {
        callback && callback.apply(mod, [null, true, 'no sid given']);
        return;
      }

      client.get(sid, function(err, info) {

        logger.info('info............', info);
        logger.info('err...........', err);
        if (err || !info) {
          logger.error('----get ERROR');
          logger.error(err);
          info = err;
          err = true;
          if (!info) {
            logger.error('Session ID invalid!');
          }

          return callback && callback.apply(mod, [null, err, info]);
        } else {
          logger.info(info);
          var data = JSON.parse(info.toString());
          data.sid = sid;
          req.sessionData = data;
        }

        logger.info('data', data);
        logger.info('info', info);
        callback && callback.apply(mod, [data, err, info]);
      });
    },

    set: function(sid, properties, req, callback) {
      var _this = this;
      this.get(sid, req, function _setGetCallback(data, err, info) {
        if (err) {
          return callback(data, err, info);
        }

        if (typeof properties !== 'object') {
          return callback(null, 'properties should be object type');
        }

        for (var prop in properties) {
          data[prop] = properties[prop];
        }

        // Save new properties to session storage
        _this.createSet(req, sid, data, callback);
      });
    },

    rmProp: function(sid, propArr, req, callback) {
      var _this = this;
      this.get(sid, req, function _setGetCallback(data, err, info) {
        if (err) {
          return callback(data, err, info);
        }

        if (!propArr.length) {
          return callback(null, 'properties array sould not be empty');
        }

        for (var i = 0; i < propArr.length; i++) {
          var prop = propArr[i];
          if (data[prop]) {
            delete data[prop];
          }
        }

        // Save new properties to session storage
        _this.createSet(req, sid, data, callback);
      });
    },

    clear: function(sid, req, callback) {
      req.sessionData = null;

      this.client.del(sid, callback);
    },

    getAllKeys: function(req, callback) {
      var client = mod._connect();

      if (process.env.NODE_ENV !== 'development') {
        logger.info('you cannot call getAllKeys when not in development mode');
        callback && callback.call(mod);
        return;
      }

      client.keys('*', function(err, keys) {
        callback && callback.call(mod, keys);
      });
    },

    clearAll: function(req, callback) {
      var client = mod._connect();

      if (process.env.NODE_ENV !== 'development') {
        logger.info('You cannot call clearAll when not in development mode');
        callback && callback.call(mod);
        return;
      }

      client.flushall(function() {
        callback && callback.apply(mod, arguments);
      });
    },
  });

  return mod;
};
