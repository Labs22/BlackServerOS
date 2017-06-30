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

var datastore = require('./');
var connect = datastore.connect;

/**!
 * ### getAllMessages(accountId, callback)
 * Retrieve all messages for given `accountId`
 *
 * Calls back with array of messages and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {accountId} accountId
 * @param {Function} callback
 */
datastore.getAllMessages = function (accountId, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'select m.message_id, m.from_account_id, \
        m.to_account_id, m.creation_time, m.ttl, \
        m.headers_ciphertext, m.payload_ciphertext, \
        a.username as to_username, b.username as from_username  \
        from message m \
        left join account a on m.to_account_id = a.account_id \
        left join account b on m.from_account_id = b.account_id \
        where \
        m.to_account_id = $1 and \
        m.deletion_time is null \
        order by m.creation_time asc',
      /*jslint multistr: true*/
      values: [
        accountId
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        callback(err);
        return;
      }

      // massage
      var records = [];
      result.rows.forEach(function (row) {
        row = datastore.util.camelizeObject(row);
        row.headersCiphertext = row.headersCiphertext.toString();
        row.payloadCiphertext = row.payloadCiphertext.toString();

        records.push(row);
      });

      callback(null, records);
    });
  });
}

/**!
 * ### getAllMetadata(accountId, callback)
 * Retrieve all message Ids & to/from usernames for given `accountId`
 *
 * Calls back with array of message Ids and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {accountId} accountId
 * @param {Function} callback
 */
datastore.getAllMetadata = function (accountId, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'select message.message_id, message.from_account_id, \
        message.to_account_id, \
        length(message.payload_ciphertext) as payload_length, \
        length(message.headers_ciphertext) as headers_length, \
        a.username as to_username, b.username as from_username  \
        from message \
        left join account a on message.to_account_id = a.account_id \
        left join account b on message.from_account_id = b.account_id \
        where \
        message.to_account_id = $1 and \
        message.deletion_time is null \
        order by message.creation_time asc',
      /*jslint multistr: false*/
      values: [
        accountId
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        callback(err);
        return;
      }

      // massage
      var records = [];
      result.rows.forEach(function (row) {
        row = datastore.util.camelizeObject(row);
        records.push(row);
      });

      callback(null, records);
    });
  });
}

/**!
 * ### getMessageById(messageId, callback)
 * Retrieve message for `messageId`
 *
 * Calls back with message and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {messageId} messageId
 * @param {Function} callback
 */
datastore.getMessageById = function (messageId, callback) {
  connect(function (client, done) {
    var query = {
      /*jslint multistr: true*/
      text: 'select * from message where \
        message_id = $1 and \
        deletion_time is null',
       /*jslint multistr: false*/
      values: [
        messageId
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        callback(err);
        return;
      }

      var message = datastore.util.camelizeObject(result.rows[0]);
      message.headersCiphertext = message.headersCiphertext.toString();
      message.payloadCiphertext = message.payloadCiphertext.toString();

      callback(null, message);
    });
  });
};
