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
 * # Container()
 *
 * ````
 * var container = new Container();
 * ````
 */
var Container = module.exports = function Container () {};

/**!
 * ### get(containerNameHmac, callback)
 * Retrieve all container records from the database for the specified `containerNameHmac`
 *
 * Adds records to container object and calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} containerNameHmac
 * @param {Function} callback
 */
Container.prototype.get = function (containerNameHmac, callback) {
  logger.info('getting container');

  var that = this;

  db.getContainerRecords(containerNameHmac, that.accountId, function (err, records) {
    if (err) {
      callback(err);
      return;
    }

    if (!records.length) {
      logger.info('container does not exist');
      callback('Container does not exist');
      return;
    }

    that.update('records', records);
    callback(null);
  });
};

/**!
 * ### getAfter(containerNameHmac, timestamp, callback)
 * Retrieve all container records from the database for the specified `containerNameHmac`
 * created after `timestamp`
 *
 * Adds records to container object and calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} containerNameHmac
 * @param {Number} timestamp
 * @param {Function} callback
 */
Container.prototype.getAfter = function (containerNameHmac, timestamp, callback) {
  logger.info('getting container');

  var that = this;

  db.getContainerRecordsAfter(containerNameHmac, that.accountId, timestamp, function (err, records) {
    if (err) {
      callback(err);
      return;
    }

    if (!records.length) {
      logger.info('no new records');
      callback('No new records');
      return;
    }

    that.update('records', records);
    callback(null);
  });
};

/**!
 * ### update()
 * Update one or a set of keys in the parent container object
 *
 * @param {String} key
 * @param {Object} value
 *
 * or
 *
 * @param {Object} input
 */
// TODO add field validation and callback
Container.prototype.update = function () {
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
