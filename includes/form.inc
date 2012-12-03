<?php

/**
 * @file
 *   Contains form-related functions for the installation profile
 */

/**
 * Provide a form to choose the default theme
 */
function orgright_client_get_theme_form(&$form_state, $url) {
  $form = array();
  drupal_set_title(st('Choose your default theme'));

  // Help message
  $form['message'] = array(
    '#type' => 'item',
    '#value' => st('Choose the initial theme for your site. At any time after the installation, the theme can be changed.'),
  );

  // Theme selector
  $form['theme'] = array(
    '#type' => 'radios',
    '#options' => array(
      'orgrightB' => _orgright_client_get_theme_option('orgrightB', st('orgRight Client Default')),
      'orgrightA' => _orgright_client_get_theme_option('orgrightA', st('Previous orgRight Client')),
    ),
    '#default_value' => ORGRIGHT_CLIENT_DEFAULT_THEME,
  );

  // Submit button
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );

  // Store the installation url for redirection post-submit
  $form['url'] = array(
    '#type' => 'value',
    '#value' => $url,
  );

  return $form;
}

/**
 * Submit handler for the theme form
 */
function orgright_client_get_theme_form_submit(&$form, &$form_state) {
  $sql = "UPDATE {system} SET status = %d WHERE type = 'theme' and name = '%s'";

  // Disable garland
  db_query($sql, 0, 'garland');

  // Enable all of the listed themes
  foreach ($form['theme']['#options'] as $theme => $label) {
    db_query($sql, 1, $theme);
  }

  // Set the selected theme as the default
  variable_set('theme_default', $form_state['values']['theme']);

  // Refresh the theme registry registry
  list_themes(TRUE);
  drupal_rebuild_theme_registry();

  // Initiate the next installation step
  variable_set('install_task', 'configure-features');

  // Redirect back to the installation page
  if (isset($form_state['values']['url'])) {
    drupal_goto($form_state['values']['url']);
  }
}

/**
 * Format a theme selection option
 *
 * @param $theme
 *   The theme system name
 * @param $name
 *   The theme full name
 * @return
 *   An HTML string to be used as a theme form option
 */
function _orgright_client_get_theme_option($theme, $name) {
  $option = '';
  $option .= "<strong>{$name}</strong>";
  $option .= "<br/>";
  $theme_dir = 'profiles/orgright_client/themes/' . $theme;
  if (is_file($theme_dir . '/screenshot.png')) {
    $option .= '<img alt="' . $name . '" title="' . $name . '" src="' . $theme_dir . '/screenshot.png">';
    $option .= "<br/>";
  }
  if (is_file($theme_dir . '/screenshot_full.png')) {
    $option .= '<ul><li><i><a href="' . $theme_dir . '/screenshot_full.png" target="_blank">';
    $option .= st('Click to view fullsize') . '</a></i></li></ul>';
    $option .= "<br/>";
  }
  return $option;
}


/**
 * Provide a form to choose which features to enable
 */
function orgright_client_get_features_form(&$form_state, $url) {
  $form = array();
  drupal_set_title(st('Choose from available features'));

  // Help message
  $form['message'] = array(
    '#type' => 'item',
    '#value' => st('The selected features will be enabled after the installation has completed. At any time, you can turn the available features on or off.'),
  );

  // Redirect URL to remain inside the installation after submission
  $form['url'] = array(
    '#type' => 'value',
    '#value' => $url,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => st('Continue'),
  );

  return $form;
}

/**
 * Submit handler for the feature choice form
 */
function orgright_client_get_features_form_submit(&$form, &$form_state) {
  // Build an array of chosen features
  $features = array();

  // Extract the selected features from the form
  foreach ($form_state['values'] as $key => $value) {
    if (substr($key, 0, 8) == 'feature-') {
      if ($value == 1) {
        $features[] = substr($key, 8);
      }
    }
  }

  // Store a temporary variable to access later
  variable_set('orgright_client_selected_features', $features);

  // Initiate the next installation step
  variable_set('install_task', 'install-orgright-client');

  // Redirect back to the installation page
  if (isset($form_state['values']['url'])) {
    drupal_goto($form_state['values']['url']);
  }
}
