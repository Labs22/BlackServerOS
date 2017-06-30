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

var should = require('chai').should();
var db = require("../lib/storage");

describe("datastore", function () {

  it("makes a connection", function (done) {
    db.connect(function () {
      done();
    });
  });

  it("selects the time", function (done) {
    db.connect(function (client) {
      client.query("select current_timestamp", function (err, result) {
        if (err) { return done(err); }

        var time = result.rows[0].now;
        time.should.be.a('Date');
        done();
      });
    });
  });

  it("finds our tables", function (done) {
    db.listTables(function (err, tables) {
      if (err) { return done(err); }
      tables.should.include.members(["account", "container", "message"]);
      done();
    });
  });

  it("gets a new ID number", function (done) {
    db.connect(function (client) {
      client.query(
        "select nextval('version_identifier')",
        function (err, result) {
          if (err) { return done(err); }
          var idNum = result.rows[0].nextval;
          idNum.should.be.ok;
          done();
        }
      );
    });
  });

  it("receives events for all rows as they stream", function (done) {
    db.connect(function (client) {
      var query = client.query("select * from generate_series(1, 100)");
      var rows = [];
      query.on('row', function (row) { rows.push(row); });
      query.on('error', function (err) { done(err); });
      query.on('end', function () {
        rows.should.have.length(100);
        /*jslint camelcase: false*/
        rows[0].generate_series.should.equal(1);
        /*jslint camelcase: true*/
        done();
      });
    });
  });
});
