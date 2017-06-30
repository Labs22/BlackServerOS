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

var app = process.app;
var middleware = require('../lib/middleware');
var verifySession = middleware.verifySession;
var Container = require('../lib/container');

/**!
 * ### GET /container/:containerNameHmac
 * Retrieve container records for the given `containerNameHmac`
*/
app.get('/container/:containerNameHmac', verifySession, function(req, res) {
  var accountId = req.session.accountId;
  var containerNameHmac = req.params.containerNameHmac;
  var after = req.query.after || 0;

  var container = new Container();
  container.update('accountId', accountId);

  container.getAfter(containerNameHmac, after, function(err) {
    if (err) {
      logger.error(err);
      res.send({
        success: false,
        error: err
      });
      return;
    }

    res.send({
      success: true,
      records: container.records
    });
  });
});

/**!
 * ### GET /container/:containerNameHmac/:recordVersionIdentifier
 * Retrieve specific record for the given `containerNameHmac` by `recordVersionIdentifier`
*/
app.get('/container/:containerNameHmac/:recordVersionIdentifier', verifySession, function (req, res) {
  var accountId = req.session.accountId;
  var containerNameHmac = req.params.containerNameHmac;
  var versionIdentifier = req.params.recordVersionIdentifier;

  var container = new Container();
  container.update('accountId', accountId);
  container.get(containerNameHmac, function (err) {
    if (err) {
      res.send({
        success: false,
        error: err
      });
      return;
    }

    // TODO this has to be a map because records is going to be an array
    if (!container.records[versionIdentifier]) {
      logger.warn('record does not exist');
      res.send({
        success: false,
        error: 'Record identifier does not exist'
      });
      return;
    }

    res.send({
      success: true,
      records: container.records[versionIdentifier] // TODO this won't work
    });
  });
});
