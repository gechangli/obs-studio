"use strict";

function simplifyLoginPage() {
    document.querySelector('.index-wrapper').remove();
    document.querySelector('.index-footer-container').remove();
    document.getElementById('panda_footer').remove();
    document.querySelector('.panda-header').remove();
    document.querySelector('.index-lift.clearfix.join-anchor').remove();
    document.querySelector('.tab.ruc-goto-register-btn').remove();
    document.querySelector('.ruc-dialog-close').remove();
    document.querySelector('.ruc-form-item.ruc-third-way-login.forgin-login').remove();
}

function onLoginPage() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        // show web view
        var lp = channel.objects.lp;
        lp.showWeb();

        // click login button to show login ui
        document.querySelector('.tool-user-info-login.header-login-btn').click();

        // simplify page
        simplifyLoginPage();
        
        // hook login button
        var btnLogin = document.querySelector('.ruc-form-item.button-container.login-button-container');
        btnLogin.onclick = function (e) {
            // get username and password
            var username = document.querySelector('.ruc-input-name.ruc-input-login-name[name="account"]').value;
            var password = document.querySelector('.ruc-input-login-passport[name="password"]').value;

            // save to c++ side
            lp.saveLivePlatformUserInfo(username, password);
        }
    });
}

function onSettingsPage() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        function startLive() {
            var lp = channel.objects.lp;

            // if start live button is visible, click it and wait for a while
            var btnStartLive = document.getElementById('start-live-btn');
            while (queryCss(btnStartLive, 'display') != 'none') {
                btnStartLive.click();
                setTimeout(startLive, 100);
            }

            // get rtmp info
            var url = document.getElementById('rtmp-address').value;
            var key = document.getElementById('live-code').value;
            lp.saveLivePlatformRtmpInfo(url, key);
            lp.closeWeb();
        }

        // first time call
        startLive();
    });
}

// dispatch
if(window.location.href.indexOf('setting') == -1) {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        var lp = channel.objects.lp;
        if(lp.m_loggedIn) {
            window.location.href = "https://www.panda.tv/setting";
        } else {
            onLoginPage();
        }
    });
} else {
    onSettingsPage();
}