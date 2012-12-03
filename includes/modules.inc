<?php

/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *   An array of modules to enable.
 */
function orgright_client_specified_modules() {
  return array(
    // Required Core modules
    //'block', 'filter', 'node', 'system', 'user',
    // Optional Core modules
    'color',
    'comment',
    'dblog',
    'help',
    'menu',
    'path',
    'search',
    'trigger',
    'update',
    'upload',
    // Optional Core modules with dependencies
    'taxonomy',
    'forum',
    // CCK
    'content',
    'text',
    'number',
    'optionwidgets',
    // Date/Time
    'date_api',
    'date_timezone',
    'date',
    'date_popup',
    // Token
    'token',
    'token_actions',
    // Print
    'print',
    'print_pdf',
    'print_mail',
    // Rules
    'rules',
    'rules_admin',
    'rules_scheduler',
    // User Interface
    'jquery_ui',
    'jquery_update',
    // Views
    'views',
    'views_ui',
    // Other
    'advanced_help',
    'admin_menu',
    'backup_migrate',
    'feedback',
    'install_profile_api',
    // orgRight
    'orgright',
    'membership',
    'committee',
    'meeting',
    'project',
    'filecabinet',
    'helpservice',
    'mailhandler',
    'maillist',
  );
}
