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

/**!
 * # Transaction()
 *
 * ````
 * var tx = new Transaction();
 * ````
 */
var Transaction = module.exports = function Transaction () {};

/**!
 * ### create(accountId, callback)
 * Attempt to add a new transaction to the database belonging to the specified `accountId`
 * Adds `transactionId` to transaction object if successful
 * Calls back with error if unsuccessful
 *
 * @param {Number} accountId
 * @param {Function} callback
 */
Transaction.prototype.create = function (accountId, callback) {
  logger.info('creating transaction');

  var that = this;
  this.update('accountId', accountId);

  db.createTransaction(accountId, function (err, id) {
    if (err) {
      callback(err);
      return;
    }

    that.update('transactionId', id);
    callback(null);
  });
};

/**!
 * ### get(transactionId, callback)
 * Attempt retreive transaction data from the database for the specified `transactionId`
 * Adds data to transaction object if successful
 * Calls back with error if unsuccessful
 *
 * @param {Number} transactionId
 * @param {Function} callback
 */
// TODO which data?
Transaction.prototype.get = function (transactionId, callback) {
  logger.info('getting transaction with id: ' + transactionId);

  var that = this;

  db.getTransaction(transactionId, function (err, transaction) {
    if (err) {
      callback(err);
      return;
    }

    if (!transaction.transactionId) {
      logger.warn('transaction does not exist');
      callback('Transaction does not exist');
      return;
    }

    that.update(transaction);
    callback(null);
  });
};

/**!
 * ### update()
 * Update one or a set of keys in the parent transaction object
 *
 * @param {String} key
 * @param {Object} value
 *
 * or
 *
 * @param {Object} input
 */
// TODO add field validation and callback
Transaction.prototype.update = function () {
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
 * ### add(data, callback)
 * Add a chunk to current transaction
 *
 * @param {Object} data
 * @param {Function} callback
 */
Transaction.prototype.add = function (data, callback) {
  logger.info('adding data to transaction');

  var that = this;

  this.assertOwnership(callback, function () {
    db.updateTransaction(that, data, callback);
  });
};

/**!
 * ### abort(callback)
 * Mark transaction as aborted with database
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Transaction.prototype.abort = function (callback) {
  logger.info('aborting transaction');

  var that = this;
  this.assertOwnership(callback, function () {
    db.abortTransaction(that.transactionId, callback);
  });
};

/**!
 * ### requestCommit(callback)
 * Request transaction commital with database
 * Calls back with error if request is unsuccessful
 *
 * @param {Function} callback
 */
Transaction.prototype.requestCommit = function (callback) {
  logger.info('requesting transaction commit');

  var that = this;
  this.assertOwnership(callback, function () {
    db.requestTransactionCommit(that.transactionId, that.accountId, callback);
  });
};

/**!
 * ### isProcessed(callback)
 * Checks database to see to if transaction has been fully committed
 * Calls back with error if request is unsuccessful
 * otherwise calls back without error, boolean finished status,
 * boolean commit success, and any commit errors
 *
 * @param {Function} callback
 */
Transaction.prototype.isProcessed = function (callback) {
  logger.info('checking transaction commit status');

  db.transactionIsProcessed(this.transactionId, callback);
};

/**!
 * ### assertOwnership(callback, next)
 * Determine if transaction's `interactingAccount` matches `accountId` from database
 * Calls `next` if successful
 * Calls `callback` with error if unsuccessful
 *
 * @param {Function} callback
 * @param {Function} next
 */
// TODO consider switching argument order
Transaction.prototype.assertOwnership = function (callback, next) {
  logger.info('asserting transaction ownership');

  if (this.interactingAccount === this.accountId) {
    next();
  } else {
    logger.warn('transaction does not belong to account');
    callback('Transaction does not belong to account');
  }
};
