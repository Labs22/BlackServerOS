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
 * # Inbox(id)
 *
 * ````
 * var inbox = new Inbox(1);
 * ````
 *
 * @param {Number} accountId
 */
var Inbox = module.exports = function Inbox (accountId) {
  this.accountId = accountId;
};

/**!
 * ### getAllMessages(callback)
 * Retrieve all messages from the database for the specified `accountId`
 *
 * Calls back without error and with messages if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Inbox.prototype.getAllMessages = function (callback) {
  db.getAllMessages(this.accountId, function (err, messages) {
    callback(err, messages);
  });
};

/**!
 * ### getAllMetadata(callback)
 * Retrieve all message Ids & to/from usernames for the specified `accountId`
 *
 * Calls back without error and with array of ids/names if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Inbox.prototype.getAllMetadata = function (callback) {
  db.getAllMetadata(this.accountId, function (err, metadata) {
    callback(err, metadata);
  });
};


/**!
 * ### getMessageById(messageId, callback)
 * Retrieve message from the database for the specified `accountId` and `messageId`
 *
 * Calls back without error and with messages if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} messageId
 * @param {Function} callback
 */
Inbox.prototype.getMessageById = function (messageId, callback) {
  var that = this;

  db.getMessageById(messageId, function (err, message) {
    if (message.toAccountId !== that.accountId) {
      // don't divulge existance
      callback('Message does not exist');
      return;
    }

    db.getAccountById(message.fromAccountId, function (err, account) {
      if (err) {
        callback('Database Error');
        return;
      }

      message.fromUsername = account.username;
      callback(err, message);
    });
  });
};
