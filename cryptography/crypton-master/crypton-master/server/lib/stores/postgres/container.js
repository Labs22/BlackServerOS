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
 * ### getContainerRecords(containerNameHmac, accountId, callback)
 * Retrieve all records for given `containerNameHmac` accessible by `accountId`
 *
 * Calls back with array of records and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} containerNameHmac
 * @param {Number} accountId
 * @param {Function} callback
 */
exports.getContainerRecords = function (containerNameHmac, accountId, callback) {
  connect(function (client, done) {
    var query = {
      // TODO limit to to_account_id
      /*jslint multistr: true*/
      text: '\
        select \
          container_record.*, \
          container_session_key.signature, \
          container_session_key_share.session_key_ciphertext \
        from container_record \
          join container_session_key using (container_session_key_id) \
          join container_session_key_share using (container_session_key_id) \
        where container_session_key.supercede_time is null \
          and container_session_key_share.deletion_time is null \
          and container_record.container_id = ( \
            select container_id from container where name_hmac = $1 \
          ) \
          and container_session_key_share.to_account_id = $2 \
        order by container_record.creation_time, container_record_id asc',
      /*jslint multistr: false*/
      values: [
        containerNameHmac,
        accountId
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        return callback(err);
      }

      // massage
      var records = [];
      result.rows.forEach(function (row) {
        Object.keys(row).forEach(function (key) {
          if (Buffer.isBuffer(row[key])) {
            row[key] = row[key].toString();
          }
        });

        row = datastore.util.camelizeObject(row);
        records.push(row);
      });

      callback(null, records);
    });
  });
};

/**!
 * ### getContainerRecordsAfter(containerNameHmac, accountId, timestamp, callback)
 * Retrieve all records for given `containerNameHmac` accessible by `accountId`
 * created after `timestamp`
 *
 * Calls back with array of records and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} containerNameHmac
 * @param {Number} accountId
 * @param {Number} timestamp
 * @param {Function} callback
 */
exports.getContainerRecordsAfter = function (containerNameHmac, accountId, timestamp, callback) {
  connect(function (client, done) {
    var query = {
      // TODO limit to to_account_id
      /*jslint multistr: true*/
      text: '\
        select \
          container_record.*, \
          container_session_key.signature, \
          container_session_key_share.session_key_ciphertext \
        from container_record \
          join container_session_key using (container_session_key_id) \
          join container_session_key_share using (container_session_key_id) \
        where container_session_key.supercede_time is null \
          and container_session_key_share.deletion_time is null \
          and container_record.container_id = ( \
            select container_id from container where name_hmac = $1 \
          ) \
          and container_session_key_share.to_account_id = $2 \
          and container_record.creation_time > to_timestamp($3) \
        order by container_record.creation_time, container_record_id asc',
      /*jslint multistr: false*/
      values: [
        containerNameHmac,
        accountId,
        timestamp / 1000
        // javascript timestamps are millisecond whereas postgres uses second granularity
      ]
    };

    client.query(query, function (err, result) {
      done();

      if (err) {
        return callback(err);
      }

      // massage
      var records = [];
      result.rows.forEach(function (row) {
        Object.keys(row).forEach(function (key) {
          if (Buffer.isBuffer(row[key])) {
            row[key] = row[key].toString();
          }
        });

        row = datastore.util.camelizeObject(row);
        records.push(row);
      });

      callback(null, records);
    });
  });
};

/**!
 * ### getContainerCreator(containerNameHmac, callback)
 * Retrieve account_id of creator of container
 * with given `containerNameHmac`
 *
 * Calls back with account_id and without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} containerNameHmac
 * @param {Function} callback
 */
exports.getContainerCreator = function (containerNameHmac, callback) {
  connect(function (client, done) {
    var query = {
      text: 'select account_id from container where name_hmac = $1',
      values: [
        containerNameHmac
      ]
    };

    client.query(query, function (err, result) {
      if (err) {
        done();
        return callback(err);
      }

      if (result.rows.length) {
        done();
        return callback(null, result.rows[0].account_id);
      } else {
        var pendingContainersQuery = {
          text: '\
            select account_id from transaction where transaction_id in ( \
              select transaction_id from transaction_add_container where name_hmac = $1)',
          values: [
            containerNameHmac
          ]
        };

        client.query(pendingContainersQuery, function (err, result) {
          done();

          if (err) {
            return callback(err);
          }

          if (!result.rows.length) {
            return callback('Could not find container author');
          }

          callback(null, result.rows[0].account_id);
        });
      }
    });
  });
};
