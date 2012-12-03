<?php

/**
 * Return a description of the profile for the initial installation screen.
 *
 * @return
 *   An array with keys 'name' and 'description' describing this profile,
 *   and optional 'language' to override the language selection for
 *   language-specific profiles.
 */
function orgright_client_profile_details() {
  return array(
    'name' => 'orgRight Client',
    'description' => 'This profile will build a site for an orgRight Client.'
  );
}

/**
 * Return an array of the modules to be enabled when this profile is installed.
 *
 * @return
 *   An array of modules to enable.
 */
function orgright_client_profile_modules() {
  orgright_client_include('modules');
  return orgright_client_specified_modules();
}

/**
 * Return a list of tasks that this profile supports.
 *
 * @return
 *   A keyed array of tasks the profile will perform during
 *   the final stage. The keys of the array will be used internally,
 *   while the values will be displayed to the user in the installer
 *   task list.
 */
function orgright_client_profile_task_list() {
  return array(
    'configure-theme' => st('Select orgRight Client theme'),
    //'configure-features' => st('Select orgRight Client features'),
    'install-orgright-client' => st('Install orgRight Client'),
  );
}

/**
 * Perform any final installation tasks for this profile.
 *
 * The installer goes through the profile-select -> locale-select
 * -> requirements -> database -> profile-install-batch
 * -> locale-initial-batch -> configure -> locale-remaining-batch
 * -> finished -> done tasks, in this order, if you don't implement
 * this function in your profile.
 *
 * If this function is implemented, you can have any number of
 * custom tasks to perform after 'configure', implementing a state
 * machine here to walk the user through those tasks. First time,
 * this function gets called with $task set to 'profile', and you
 * can advance to further tasks by setting $task to your tasks'
 * identifiers, used as array keys in the hook_profile_task_list()
 * above. You must avoid the reserved tasks listed in
 * install_reserved_tasks(). If you implement your custom tasks,
 * this function will get called in every HTTP request (for form
 * processing, printing your information screens and so on) until
 * you advance to the 'profile-finished' task, with which you
 * hand control back to the installer. Each custom page you
 * return needs to provide a way to continue, such as a form
 * submission or a link. You should also set custom page titles.
 *
 * You should define the list of custom tasks you implement by
 * returning an array of them in hook_profile_task_list(), as these
 * show up in the list of tasks on the installer user interface.
 *
 * Remember that the user will be able to reload the pages multiple
 * times, so you might want to use variable_set() and variable_get()
 * to remember your data and control further processing, if $task
 * is insufficient. Should a profile want to display a form here,
 * it can; the form should set '#redirect' to FALSE, and rely on
 * an action in the submit handler, such as variable_set(), to
 * detect submission and proceed to further tasks. See the configuration
 * form handling code in install_tasks() for an example.
 *
 * Important: Any temporary variables should be removed using
 * variable_del() before advancing to the 'profile-finished' phase.
 *
 * @param $task
 *   The current $task of the install system. When hook_profile_tasks()
 *   is first called, this is 'profile'.
 * @param $url
 *   Complete URL to be used for a link or form action on a custom page,
 *   if providing any, to allow the user to proceed with the installation.
 *
 * @return
 *   An optional HTML string to display to the user. Only used if you
 *   modify the $task, otherwise discarded.
 */
function orgright_client_profile_tasks(&$task, $url) {
  orgright_client_include('tasks');
  return orgright_client_process_tasks($task, $url);
}

/**
 * Helper function to load include files
 *
 * @param $name
 *   The file name without the .inc extension
 * @param $dir
 *   The directory containing the include file
 */
function orgright_client_include($name, $dir = 'includes') {
  require_once("profiles/orgright_client/{$dir}/{$name}.inc");
}

/**
 * Implementation of hook_form_FORMID_alter().
 *
 * Allows the profile to alter the site-configuration form. This is
 * called through custom invocation, so $form_state is not populated.
 * Additionally, the standard hook invocation is NOT used and so only
 * a limited number of modules get called.  Unfortunately the "profile"
 * is not one of them so pretend to be the "system" module.  This will
 * cause problems if more that one profile is doing it.
 */

/**
 * Alter the install profile selection form
 */
function system_form_install_select_profile_form_alter(&$form, $form_state) {
  foreach ($form['profile'] as $key => $element) {
    // Set orgright_client as the default
    $form['profile'][$key]['#value'] = 'orgright_client';
  }
}

/**
 * Alter the install settings form (database selection)
 *
 */
function system_form_install_settings_form_alter(&$form, $form_state) {
  // nothing to do yet

}

/**
 * Alter the install profile configuration
 */
function system_form_install_configure_form_alter(&$form, $form_state) {
  // Set default for site name field.
  $form['site_information']['site_name']['#default_value'] = $_SERVER['SERVER_NAME'];
  // Add option to turn on forced login
  $form['site_information']['user_force_login'] = array(
    '#type' => 'checkbox',
    '#title' => t('Force users to login'),
    '#description' => t('If checked, users will be required to log into the site to access it. Users who are not logged in will be redirected to a login page. Select this setting if your orgright site must be closed to the public, such as a company intranet.'),
  );

  // Add timezone options required by date (Taken from Open Atrium)
  if (function_exists('date_timezone_names') && function_exists('date_timezone_update_site')) {
    $form['server_settings']['date_default_timezone']['#access'] = FALSE;
    $form['server_settings']['#element_validate'] = array('date_timezone_update_site');
    $form['server_settings']['date_default_timezone_name'] = array(
      '#type' => 'select',
      '#title' => t('Default time zone'),
      '#default_value' => 'Pacific/Auckland',
      '#options' => date_timezone_names(FALSE, TRUE),
      '#description' => st('Select the default site time zone. If in doubt, choose the timezone that is closest to your location which has the same rules for daylight saving time.'),
      '#required' => TRUE,
    );
  }

  // Add an additional submit handler to process the form
  $form['#submit'][] = 'orgright_client_install_configure_form_submit';
}

/**
 * Submit handler for the installation configure form
 */
function orgright_client_install_configure_form_submit(&$form, &$form_state) {
  variable_set('orgright_client_force_login', $form_state['values']['user_force_login']);
}

