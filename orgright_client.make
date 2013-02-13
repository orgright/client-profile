; Drush make file for orgRight Client install profile release 2.2
core = 6.x

api = 2

; Contrib Modules
projects[admin_menu] = "1.8"
projects[advanced_help] = "1.2"
projects[backup_migrate] = "2.6"
projects[cck] = "2.9"
projects[date] = "2.9"
projects[install_profile_api] = "2.2"
projects[jquery_ui] = "1.5"
projects[jquery_update] = "2.0-alpha1"
projects[memcache] = "1.10"
projects[print] = "1.18"
projects[rules] = "1.5"
projects[token] = "1.19"
projects[views] = "2.16"

; Custom Modules
; Please fill the following out. Type may be one of get, cvs, git, bzr or svn,
; and url is the url of the download.
projects[org_modules][type] = "module"
projects[org_modules][download][type] = "get"
projects[org_modules][download][url] = "https://github.com/orgright/custom-modules/archive/6.x-2.2.tar.gz"


; Contrib Themes
projects[zen] = "2.1"

; Custom Themes
; Please fill the following out. Type may be one of get, cvs, git, bzr or svn,
; and url is the url of the download.
projects[org_themes][type] = "theme"
projects[org_themes][download][type] = "get"
projects[org_themes][download][url] = "https://github.com/orgright/custom-themes/archive/6.x-2.0.tar.gz"

; External libraries
libraries[jquery_ui][dowmload][type] = "file"
libraries[jquery_ui][download][url] = "http://jquery-ui.googlecode.com/files/jquery-ui-1.7.3.zip"
libraries[jquery_ui][directory_name] = "jquery.ui"

