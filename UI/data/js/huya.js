"use strict";

function SimplifyLoginPage() {
    document.getElementById('duya-header').remove()
    document.querySelector('.huya-footer').remove()
    document.querySelector('.login-sidebar').remove()
    document.getElementById('js-login-close').remove()
    document.querySelector('.login-wran-tip').remove()
    document.querySelector('.login-hd').remove()
}

if(document.cookie.indexOf('username=') == -1) {
    // need login, simplify login page
    SimplifyLoginPage()
} else {

}