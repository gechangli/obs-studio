"use strict";

function simplifyLoginPage() {
    document.getElementById('duya-header').remove();
    document.querySelector('.huya-footer').remove();
    document.querySelector('.login-sidebar').remove();
    document.getElementById('js-login-close').remove();
    document.querySelector('.login-wran-tip').remove();
    document.querySelector('.login-hd').remove();
}

function amIAnchor() {
    var list = document.querySelectorAll('.nav-section>.header');
    for(var i in list) {
        var text = list[i].innerText;
        if(text != undefined && text.indexOf('\u6211\u662f\u4e3b\u64ad') != -1) { // 我是主播
            return true;
        }
    }
    return false;
}

function grabUrl() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        var save = function() {
            var url = document.getElementById('copyLink').value;
            if(url == '/') {
                setTimeout(save, 200);
            } else {
                var username = getCookie("username");
                if(username.indexOf('yy') == username.length - 2) {
                    username = username.substring(0, username.length - 2);
                }
                var lp = channel.objects.lp;
                lp.SaveLivePlatformRtmpInfo(url, "");
                lp.SaveLivePlatformUserInfo(username, "");
                lp.CloseWeb();
            }
        };
        save();
    });
}

function onLoginPage() {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        var lp = channel.objects.lp;
        lp.ShowWeb();
        simplifyLoginPage();
    });
}

function onSettingsPage() {
    // if I am anchor, open rtmp
    // if I am not, prompt user
    if(amIAnchor()) {
        // function to enable rtmp
        var gotoRTMPDone = false;
        var enableRTMPDone = false;
        var openRtmp = function() {
            // check live is started or not by check getRtmpWrap element style
            // none means stream push url is opened, so we save url
            var wrap = document.getElementById('getRtmpWrap');
            var isWrapInvisible = queryCss(wrap, 'display') == 'none';
            if(isWrapInvisible) {
                grabUrl();
                return;
            }

            // goto rtmp section
            if(!gotoRTMPDone) {
                var link = document.getElementById('openRTMP');
                if (link != null) {
                    var a = link.children[0];
                    if(a != null) {
                        a.click();
                        gotoRTMPDone = true;
                    }
                }
            }

            // delay for a while to ensure rtmp section is shown
            setTimeout(function () {
                // enable rtmp
                if(gotoRTMPDone && !enableRTMPDone) {
                    isWrapInvisible = queryCss(wrap, 'display') == 'none';
                    var link = document.querySelector('.clickstat.get_rtmp.active');
                    if (link != null && !isWrapInvisible) {
                        link.click();
                        enableRTMPDone = true;
                    }
                }

                // check done or not
                // if not done, redo after a while
                if(!gotoRTMPDone || !enableRTMPDone) {
                    setTimeout(openRtmp, 200);
                }
            }, 1000);
        };

        // call first time
        openRtmp();
    } else {
        // close web and prompt user to open live
        new QWebChannel(qt.webChannelTransport, function(channel) {
            var lp = channel.objects.lp;
            lp.ShowMessageBox("\u672a\u5f00\u901a\u76f4\u64ad", "\u8bf7\u5148\u5f00\u901a\u4e3b\u64ad\u6743\u9650"); // "未开通直播", "请先开通主播权限"
            lp.ClearCookies();
            lp.CloseWeb();
        });
    }
}

function onOtherPage() {
    window.location.href = 'http://i.huya.com/index.php?m=ProfileSetting';
}

// dispatch
if(getCookie('username') == null) {
    onLoginPage();
} else if(window.location.href.indexOf('i.huya.com') != -1) {
    onSettingsPage();
} else {
    onOtherPage();
}