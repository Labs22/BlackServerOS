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

describe('Container functionality', function () {
  this.timeout(15000);

  var session;

  before(function (done) {
    crypton.authorize('notSoSmart', 'pass', function (err, rawSession) {
      if (err) throw err;
      session = rawSession;
      done();
    });
  });

  describe('save()', function () {
    it('should save container changes', function (done) {
      session.create('tupperware', function (err, container) {
        assert.equal(err, null);
        container.keys.color = 'blue';

        container.save(function (err) {
          assert.equal(err, null);

          crypton.authorize('notSoSmart', 'pass', function (err, session2) {
            assert.equal(err, null);

            session2.load('tupperware', function (err, container2) {
              assert.equal(err, null);
              assert.equal(container2.keys.color, 'blue');
              done();
            });
          });
        });
      });
    });
  });

  describe('getHistory()', function () {
    it('should get container records', function (done) {
      session.load('tupperware', function (err, container) {
        assert.equal(err, null);

        container.getHistory(function (err, records) {
          assert.equal(err, null);

          // TODO is there a better way to test the integrity of the response?
          assert.equal(records[0].accountId, 1);
          done();
        });
      });
    });
  });

  describe('sync()', function () {
    it('should pull changes into an instantiated container', function (done) {
      // force the session to load from server
      session.containers = [];

      session.load('tupperware', function (err, container) {
        assert.equal(err, null);

        // force the session to load from server again
        session.containers = [];

        session.load('tupperware', function (err, container2) {
          assert.equal(err, null);
          container.keys.color = 'green';

          container.save(function (err) {
            assert.equal(err, null);

            container2.sync(function (err) {
              assert.equal(err, null);
              assert.equal(container2.keys.color, 'green');
              done();
            });
          });
        });
      });
    });
  });
});
