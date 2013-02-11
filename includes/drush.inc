<?php

/**
 * @file
 *   Functions to aid drush-based installations
 */

/**
 * Automated run through the option forms since drush can't
 * handle the forms
 *
 * @param &$task
 *   The installation profile task
 */
function orgright_client_drush_tasks(&$task) {
  orgright_client_include('form');

  if ($task == 'configure-theme') {
    // Set the theme
    // build the theme selection form
    $form_state = array();
    $url = '';
    $form = orgright_client_get_theme_form($form_state, $url);

    // specify the selected values
    $form_state['values']['theme'] = ORGRIGHT_CLIENT_DEFAULT_THEME;

    // and call the submit function
    orgright_client_get_theme_form_submit($form, $form_state);
    // submit function will set task to 'configure-features'
  }

  $task = variable_get('install_task', '');
  if ($task == 'configure-features') {
    // nothing to do, so move onto next step
    $task = 'install-orgright-client';
  }

  // Have skipped the forms, so continue with the installation
  variable_set('install_task', $task);
  return $task;
}
