"use strict";

function simplifyLoginPage() {
    document.getElementById("header").remove();
    document.getElementById("footer").remove();
    document.querySelector('.mainbody').setAttribute('style',
        "width: 300px;" +
        "min-width: 300px;" +
        "background-size: 0;" +
        "background: none;" +
        "height: auto;" +
        "margin: 0;" +
        "padding: 0;" +
        "left: 50%;");
    document.getElementById('loginbox').setAttribute('style', 'margin:0 auto;');
    document.querySelector('.hot-ad-con').remove();
    document.querySelector('.loginNormal').classList.remove('hide');
    document.querySelector('.scancode-login').remove();
    document.querySelector('.scanicon-switch-box').remove();
    document.querySelector('.third-text').remove();
    document.querySelector('.third-list.clearfix').remove();
    document.querySelector('.loginbox-noact.clearfix').remove();
    document.querySelector('.login-box-toptxt').remove();
    new QWebChannel(qt.webChannelTransport, function(channel) {
        var lp = channel.objects.lp;
        document.querySelector('.wrapper.sign-con').setAttribute('style',
            'width:' + lp.m_pageWidth + 'px;');
    })
}

function simplifySettingsPage() {
    document.getElementById("header").remove();
    document.querySelector('.u_header').remove();
    document.querySelector('.u_nav').remove();
    var controlGroups = document.querySelectorAll('.control_group.clearfix');
    for(var i = controlGroups.length - 1; i >= 0; i--) {
        controlGroups[i].remove();
    }
    document.querySelectorAll('.primary_button01.btn_big')[1].remove();
    document.querySelector('.fglo_bg').remove();
    var mainBody = document.querySelector('.u_mainbody');
    mainBody.classList.remove('u_mainbody');
    mainBody.setAttribute('style', 'width:200px;');
    document.querySelector('.live_wrap.clearfix').setAttribute('style', 'width:200px;');
}

function onLoginPage() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        // show web view
        var lp = channel.objects.lp;
        lp.showWeb();

        // simplify html
        simplifyLoginPage();

        // hook login button to save username/password when click
        var btnLogin = document.querySelector('.loginbox-sbt.btn-sub[type="submit"]');
        btnLogin.onclick = function (e) {
            // check login by phone or nickname
            var tabPhone = document.querySelector('.l-stype.js-l-stype[data-subtype="sub.phonenum"]');
            var phoneLogin = tabPhone.classList.contains('active');
            var username = "";
            var password = "";
            if(phoneLogin) {
                username = document.querySelector('.ipt[name="phoneNum"]').value;
            } else {
                username = document.querySelector('.ipt[name="username"]').value;
            }
            password = document.querySelector('.ipt[name="password"]').value;

            // save to c++ side
            lp.saveLivePlatformUserInfo(username, password);
        };
    });
}

function onSettingsPage() {
    // check cookie to see if user enables broadcast
    // if acf_own_room is 1, then broadcast is enabled
    if (getCookie('acf_own_room') == '1') {
        // check if already in living state, we can check rtmp_url selector
        if(document.querySelector('#rtmp_url') == null) {
            // simplify page
            simplifySettingsPage();

            // auto click open
            var btnOpenLive = document.getElementById('js_start_show');
            btnOpenLive.click();

            // reload current page after a while
            setTimeout(function() {
                window.location.href = "https://www.douyu.com/room/my";
            }, 1000);
        } else {
            // get rtmp url and push code, set it back
            new QWebChannel(qt.webChannelTransport, function(channel) {
                var url = document.querySelector('#rtmp_url').value;
                var key = document.querySelector('#rtmp_val').value;
                var lp = channel.objects.lp;
                lp.saveLivePlatformRtmpInfo(url, key);
                lp.closeWeb();
            });
        }
    } else {
        // close web and prompt user to open live
        new QWebChannel(qt.webChannelTransport, function(channel) {
            var lp = channel.objects.lp;
            lp.showMessageBox("\u672a\u5f00\u901a\u76f4\u64ad", "\u8bf7\u5148\u5f00\u901a\u4e3b\u64ad\u6743\u9650"); // "未开通直播", "请先开通主播权限"
            lp.clearCookies();
            lp.closeWeb();
        })
    }
}

function onOtherPage() {
    window.location.href = "https://www.douyu.com/room/my";
}

// dispatch
try {
    if (window.location.href.indexOf("login") > 0) {
        onLoginPage();
    } else if (window.location.href.indexOf("room/my") > 0) {
        onSettingsPage();
    } else {
        onOtherPage();
    }
} catch(e) {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        var lp = channel.objects.lp;
        lp.jsLog(e.stack);
        lp.closeWeb();
    })
}