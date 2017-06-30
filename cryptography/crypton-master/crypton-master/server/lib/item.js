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
 * # Item()
 *
 * ````
 * var item = new Item();
 * ````
 */
var Item = module.exports = function Item () {};

/**!
 * ### get(itemNameHmac, callback)
 * Retrieve value from the database for the specified `itemNameHmac`
 *
 * Adds value to item object and calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {String} itemNameHmac
 * @param {Function} callback
 */
Item.prototype.get = function (itemNameHmac, callback) {
  logger.info('getting item');

  var that = this;

  db.getItemValue(itemNameHmac, that.accountId, function (err, record) {
    if (err) {
      callback(err);
      return;
    }

    that.update('rawData', record);
    callback(null);
  });
};

/**!
 * ### update()
 * Update one or a set of keys in the item object
 *
 * @param {String} key
 * @param {Object} value
 *
 * or
 *
 * @param {Object} input
 */
Item.prototype.update = function () {
  // XXXddahl: validate object keys/values against 'valid keys'
  if (typeof arguments[0] === 'object') {
    for (var key in arguments[0]) {
        if (arguments[0].hasOwnProperty(key)) {
            this[key] = arguments[0][key];
        }
    }
  }

  // update('key', 'value')
  else if (typeof arguments[0] === 'string' && arguments[1]) {
    this[arguments[0]] = arguments[1];
  }
};

/**!
 * ### save()
 * Save the current state of the Item
 *
 * @param {Function} callback
 *
 */
Item.prototype.save = function item_save(callback) {
  logger.info('Saving item');

  var that = this;
  db.saveItem(that.itemNameHmac, that.accountId,
	      that.value, that.timelineVisible, function (err, result) {
    if (err) {
      callback(err);
      return;
    }

    callback(null, result);
  });
};

/**!
 * ### create()
 * Create an Item
 *
 * @param {Function} callback
 *
 */
Item.prototype.create = function item_create(callback) {
  logger.info('creating item');

  var that = this;
  db.createItem(that.itemNameHmac, that.accountId,
                that.value, that.wrappedSessionKey,
  function (err, itemMetaData) {
    if (err) {
      callback(err);
      return;
    }
    callback(null, itemMetaData);
  });
};

/**!
 * ### remove()
 * Remove an Item
 *
 * @param {Function} callback
 *
 */
Item.prototype.remove = function item_remove(callback) {
  logger.info('remove item');

  var that = this;
  db.removeItem(that.itemNameHmac, that.accountId,
  function (err, result) {
    if (err) {
      callback(err);
      return;
    }
    callback(null, result);
  });
};

/**!
 * ### share()
 * Share an Item
 *
 * @param {Number} toAccountId
 * @param {Function} callback
 *
 */
Item.prototype.share = function item_share(callback) {
  logger.info('share item');

  var that = this;
  db.shareItem(that.itemNameHmac, that.sessionKeyCiphertext,
               that.toUsername, that.accountId,
  function (err, result) {
    if (err) {
      callback(err);
      return;
    }
    callback(null, result);
  });
};

/**!
 * ### unshare()
 * unshare an Item
 *
 * @param {Function} callback
 *
 */
Item.prototype.unshare = function item_unshare(callback) {
  logger.info('unshare item');

  var that = this;
  db.unshareItem(that.itemNameHmac,  that.accountId, that.shareeUsername,
  function (err, result) {
    if (err) {
      logger.error(err);
      callback(err);
      return;
    }
    callback(null, result);
  });
};

/**!
 * ### getAuthorItems(accountId, lastItemReadId, offset, limit, callback)
 * Retrieve items that an author created
 *
 * Gets specified 'history' Items calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Number} lastItemReadId
 * @param {Number} offset
 * @param {Number} limit
 * @param {Function} callback
 */
Item.prototype.getAuthorItems =
function getAuthorItems(callback) {
  var that = this;
  logger.info('getting author items from ' + that.lastItemRead);

  db.getAuthorItems(that.accountId, that.lastItemRead, that.offset, that.limit, function (err, rows) {
    if (err) {
      callback(err);
      return;
    }

    that.update('rows', rows);
    callback(null);
  });
};

/**!
 * ### getTimeline(callback)
 * Retrieve user timeline
 *
 * Gets specified Timeline rows, calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Item.prototype.getTimeline =
function getTimeline(callback) {
  var that = this;
  logger.info('getting timeline from ' + that.lastItemRead);

  db.getTimelineItems(that.accountId, that.lastItemRead, that.offset, that.limit, that.direction, function (err, rows) {
    if (err) {
      callback(err);
      return;
    }

    that.update('rows', rows);
    callback(null);
  });
};

/**!
 * ### getLatestTimelineItems(callback)
 * Retrieve latest user timeline items
 *
 * Gets specified Timeline rows, calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Item.prototype.getLatestTimelineItems =
function getLatestTimelineItems(callback) {
  var that = this;

  db.getLatestTimelineItems(that.accountId, that.limit, function (err, rows) {
    if (err) {
      callback(err);
      return;
    }

    that.update('rows', rows);
    callback(null);
  });
};

/**!
 * ### getTimelineItemsBefore(callback)
 * Retrieve timeline items before 'beforeId'
 *
 * Gets specified Timeline rows, calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Item.prototype.getTimelineItemsBefore =
function getTimelineItemsBefore(callback) {
  var that = this;

  db.getTimelineItemsBefore(that.accountId, that.limit, that.beforeId, function (err, rows) {
    if (err) {
      callback(err);
      return;
    }

    that.update('rows', rows);
    callback(null);
  });
};

/**!
 * ### getTimelineItemsAfter(callback)
 * Retrieve timeline items after 'afterId'
 *
 * Gets specified Timeline rows, calls back without error if successful
 *
 * Calls back with error if unsuccessful
 *
 * @param {Function} callback
 */
Item.prototype.getTimelineItemsAfter =
function getTimelineItemsAfter(callback) {
  var that = this;

  db.getTimelineItemsAfter(that.accountId, that.limit, that.afterId, function (err, rows) {
    if (err) {
      callback(err);
      return;
    }

    that.update('rows', rows);
    callback(null);
  });
};
