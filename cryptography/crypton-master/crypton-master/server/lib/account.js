/* Crypton Server, Copyright 2013 SpiderOak, Inc.
 *
 * This file is part of Crypton Server.
 *
 * Crypton Server is free software: you can redistribute it and/or modify it
 * under the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Crypton Server is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the Affero GNU General Public
 * License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with Crypton Server.  If not, see <http://www.gnu.org/licenses/>.
 */

'use strict';

var app = require('../app');
var db = app.datastore;
var config = app.config;
var srp = require('srp');
var validator = require('validator');

/**!
 * # Account()
 *
 * ````
 * var account = new Account();
 * ````
 */
var Account = module.exports = function Account() {};

/**!
 * ### get(username, callback)
 * Retrieve a user account from the database using the specified `username`
 *
 * Adds data to account object and calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} username
 * @param {Function} callback
 */
Account.prototype.get = function(username, callback) {
  logger.info('getting account for username: ' + username);

  var _this = this;

  db.getAccount(username, function(err, account) {
    if (err) {
      callback(err);
      return;
    }

    _this.update(account);
    callback(null);
  });
};

/**!
 * ### getById(id, callback)
 * Retrieve a user account from the database using the specified `id`
 *
 * Adds data to account object and calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} id
 * @param {Function} callback
 */
Account.prototype.getById = function(id, callback) {
  logger.info('getting account for id: ' + id);

  var _this = this;

  db.getAccountById(id || this.id, function(err, account) {
    if (err) {
      callback(err);
      return;
    }

    _this.update(account);
    callback(null);
  });
};

/**!
 * ### beginSrp()
 * Generate SRP b value, then continue calculation
 * in continueSrp()
 *
 * SRP variables are named as defined in RFC 5054
 * and RFC 2945, prefixed with 'srp'
 *
 * @param {String} srpA
 * @param {Function} callback
 */
Account.prototype.beginSrp = function(srpA, callback) {
  // TODO: 512 byte length will change when a different SRP group is used
  if (typeof srpA !== 'string' || srpA.length !== 512) {
    callback('Invalid SRP A value.');
    return;
  }

  var _this = this;
  srp.genKey(function(err, srpb) {
    if (err) {
      callback(err);
      return;
    }

    _this.continueSrp(srpA, srpb, callback);
  });
};

/**!
 * ### continueSrp()
 * Continue initial SRP calculation with supplied A
 * and generated b parameters.  This is split from
 * beginSrp() to allow testing with a supplied b
 * parameter.
 *
 * @param {String} A
 * @param {Buffer} b
 * @param {Function} callback
 */
Account.prototype.continueSrp = function(srpA, srpb, callback) {
  var verifier = new Buffer(this.srpVerifier, 'hex');
  var srpServer = new srp.Server(srp.params[2048], verifier, srpb);
  try {
    srpServer.setA(new Buffer(srpA, 'hex'));
  } catch (e) {
    callback('srpA is invalid');
    return;
  }

  callback(null, {
    b: srpb.toString('hex'),
    B: srpServer.computeB().toString('hex'),
    A: srpA,
  });
};

/**!
 * ### checkSrp()
 * Finishes SRP verification with client's M1 value.
 *
 * Calls back without error on success.
 * Calls back with an error on failure.
 *
 * @param {String} srpM1
 * @param {Function} callback
 */
Account.prototype.checkSrp = function(srpParams, srpM1, callback) {

  var srpM2;

  if (typeof srpParams !== 'object' || !srpParams.b || !srpParams.A) {
    callback('Invalid srpParams.');
    return;
  }

  if (typeof srpM1 !== 'string' || srpM1.length !== 64) {
    callback('Invalid SRP M1.');
    return;
  }

  // Revivify srpServer
  var verifier = new Buffer(this.srpVerifier, 'hex');
  var srpb = new Buffer(srpParams.b, 'hex');
  var srpServer = new srp.Server(srp.params[2048], verifier, srpb);
  var srpA = new Buffer(srpParams.A, 'hex');
  srpServer.setA(srpA);

  try {
    srpM2 = srpServer.checkM1(new Buffer(srpM1, 'hex'));
  } catch (e) {
    callback('Incorrect password');
    logger.info('SRP verification error: ' + e.toString());
    return;
  }

  // Don't need this right now. Maybe later?
  //var srpK = srpServer.computeK();

  callback(null, srpM2);
};

/**!
 * ### update(key, value)
 * Update one or a set of keys in the parent account object
 *
 * @param {String} key
 * @param {Object} value
 *
 * or
 *
 * @param {Object} input
 */
Account.prototype.update = function() {
  // TODO add field validation and callback

  // update({ key: 'value' });
  if (typeof arguments[0] === 'object') {
    for (var key in arguments[0]) {
      if (arguments[0].hasOwnProperty(key)) {
        this[key] = arguments[0][key];
      }

    }
  }

  // update('key', 'value')
  else if (typeof arguments[0] === 'string' && typeof arguments[1] !== 'undefined') {
    this[arguments[0]] = arguments[1];
  }
};

/**!
 * ### toJSON()
 * Dump non-function values of account object into an object
 *
 * @return {Object} account
 */
Account.prototype.toJSON = function() {
  var fields = {};

  for (var i in this) {
    if (typeof this[i] !== 'function') {
      fields[i] = this[i];
    }
  }

  return fields;
};

/**!
 * ### save(callback)
 * Saves the current state of the account object to the database
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Account.prototype.save = function(callback) {
  logger.info('saving account');

  var _this = this;

  function _saveUser() {
    if (!_this.username) {
      return callback('undefined is not a valid username');
    }

    // TODO: additional validation on any other account properties that need validation
    if (_this.username.length > 32) {
      return callback('Username is not valid: exceeds 32 charcters!');
    }

    if (!validator.isAlphanumeric(_this.username)) {
      return callback('Username is not valid: it is not alphanumeric!');
    }

    db.saveAccount(_this.toJSON(), callback);
  }

  return _saveUser();

};

/**!
 * ### sendMessage(options, callback)
 * Send a message from current account
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} options
 * @param {Function} callback
 */
Account.prototype.sendMessage = function(options, callback) {
  // TODO is this necessary if we're just passing `options` into db.saveMessage?
  if (!this.accountId) {
    logger.warn('accountId was not supplied');
    callback('Recipient account object must have accountId');
    return;
  }

  var toAccountId = this.accountId;

  logger.info('saving message for account id: ' + toAccountId);

  db.saveMessage(options, function(err, messageId) {
    if (err) {
      callback('Database error');
      return;
    }

    // there is definitely a better way to get the username to the recipient
    var sender = new Account();
    sender.getById(options.fromAccountId, function(err) {
      // FIXME : 'err' is never even checked!
      if (app.clients[toAccountId]) {
        logger.info('sending message over websocket');

        app.clients[toAccountId].emit('message', {
          messageId: messageId,
        });
      }
    });

    callback(null, messageId);
  });
};

/**!
 * ### changePassphrase(options, callback)
 * Change user's passphrase
 *
 * Calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Object} keyring
 * @param {Function} callback
 */
Account.prototype.changePassphrase = function(keyring, callback) {
  if (typeof keyring !== 'object') {
    return callback('changePassphrase failed, keyring object required.');
  }

  db.updateKeyring(keyring, callback);
};
