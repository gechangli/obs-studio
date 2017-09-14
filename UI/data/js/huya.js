"use strict";

function simplifyLoginPage() {
    document.getElementById('duya-header').remove();
    document.querySelector('.huya-footer').remove();
    document.querySelector('.login-sidebar').remove();
    document.getElementById('js-login-close').remove();
    document.querySelector('.login-wran-tip').remove();
    document.querySelector('.login-hd').remove();
}

function queryCss(o, key) {
    return o.currentStyle? o.currentStyle[key] : document.defaultView.getComputedStyle(o, false)[key];
}

if(document.cookie.indexOf('username=') == -1) {
    // need login, simplify login page
    simplifyLoginPage();
} else {
    // check live is started or not by check getRtmpWrap element style
    var ele = document.getElementById('getRtmpWrap');
    var display = queryCss(ele, 'display');
    if(display == 'none') {
        // none means stream push url is opened, so we save url
        new QWebChannel(qt.webChannelTransport, function(channel) {
            var url = document.getElementById('copyLink').value;
            var lp = channel.objects.lp;
            lp.GrabLivePlatformInfo(url, "");
            lp.CloseWeb();
        })
    } else {
        // function to enable rtmp
        var gotoRTMPDone = false;
        var enableRTMPDone = false;
        var openRtmp = function() {
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
            if(gotoRTMPDone && !enableRTMPDone) {
                var link = document.querySelector('.clickstat.get_rtmp.active');
                if (link != null) {
                    link.click();
                    enableRTMPDone = true;
                }
            }

            // check done or not
            // if not done, redo after 1 second
            // if yes, save url
            if(!gotoRTMPDone || !enableRTMPDone) {
                setTimeout(openRtmp, 1000);
            } else {
                new QWebChannel(qt.webChannelTransport, function(channel) {
                    var url = document.getElementById('copyLink').value;
                    if(url != "/") {
                        var lp = channel.objects.lp;
                        lp.GrabLivePlatformInfo(url, "");
                        lp.CloseWeb();
                    }
                })
            }
        };

        // call first time
        openRtmp();
    }
}