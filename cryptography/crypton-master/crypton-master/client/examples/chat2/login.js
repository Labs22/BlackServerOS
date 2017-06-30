(function () {

$(document).ready(function () {
  $('#login input')[0].focus();

  $('#login form').submit(function (e) {
    e.preventDefault();

    $('input').blur();

    var action = $('input[type=submit][clicked=true]').val().toLowerCase();
    var data = $(this).serializeArray();
    var username = data[0].value;
    var password = data[1].value;

    if (!username || !password) {
      setStatus('Please fill in the form');
      return;
    }

    actions[action](username, password);
  });

  $('form input[type=submit]').click(function() {
    $('input[type=submit]', $(this).parents('form')).removeAttr('clicked');
    $(this).attr('clicked', 'true');
  });
});

var actions = {};

actions.login = function (username, password) {
  setStatus('Logging in...');

  crypton.authorize(username, password, function (err, session) {
    if (err) {
      setStatus(err);
      return;
    }

    hideLogin();
    app.init(session);
  });
};

actions.register = function (username, password) {
  setStatus('Generating account...');

  crypton.generateAccount(username, password, function (err, account) {
    if (err) {
      setStatus(err);
      return;
    }

    setStatus('Saving account...');
    account.save(function (err) {
      if (err) {
        setStatus(err);
        return;
      }

      actions.login(username, password);
    });
  }, {
    save: false
  });
};

function setStatus (message) {
  $('#login span').text(message);
}

function hideLogin () {
  $('#login').css({
    top: '-500px'
  });

  $('body').css({
    background: '#fff'
  });
}

})();
