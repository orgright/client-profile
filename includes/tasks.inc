<?php

// Define the default WYSIWYG editor
define('ORGRIGHT_CLIENT_EDITOR', 'ckeditor');

// Define the default themes
define('ORGRIGHT_CLIENT_DEFAULT_THEME', 'orgrightB');
define('ORGRIGHT_CLIENT_ADMIN_THEME', 'garland');

/**
 * This function performs the real work on the profile install tasks
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
function orgright_client_process_tasks(&$task, $url) {
  // first include any defined helper functions from module install_profile_api
  install_include(orgright_client_profile_modules());

  $output = '';
  // Complete basic configuration then skip to the configure task
  if ($task == 'profile') {
    $output .= orgright_client_profile_task_profile($url);
    $task = 'configure-theme';
  }

  // If we're using Drush to install, skip the following forms
  if (defined('DRUSH_BASE_PATH')) {
    orgright_client_include('drush');
    orgright_client_drush_tasks($task);
  }

  // Provide a form to choose the theme
  if ($task == 'configure-theme') {
    orgright_client_include('form');
    $output = drupal_get_form('orgright_client_get_theme_form', $url);
  }

  // Provide a form to choose features
  if ($task == 'configure-features') {
    // Nothing to do here, so skip
    $task = 'install-orgright-client';
    //orgright_client_include('form');
    //$output .= drupal_get_form('orgright_client_get_features_form', $url);
  }

  // Installation batch process
  if ($task == 'install-orgright-client') {
    // Prepare a list of installation operations
    $operations = array();

    // Pre-installation operations
    $operations[] = array('orgright_client_build_directories', array());
    $operations[] = array('orgright_client_config_taxonomy', array());

    // Post-installation operations
    $operations[] = array('orgright_client_config_blocks', array());
    $operations[] = array('orgright_client_config_menus', array());
    $operations[] = array('orgright_client_config_backup_migrate', array());
    $operations[] = array('orgright_client_config_filters', array());
    //$operations[] = array('orgright_client_config_password', array());
    //$operations[] = array('orgright_client_config_wysiwyg', array());
    //$operations[] = array('orgright_client_config_ur', array());
    //$operations[] = array('orgright_client_config_views', array());
    //$operations[] = array('orgright_client_config_images', array());
    //$operations[] = array('orgright_client_config_tidy_node_links', array());

    // Create the standard orgRight users, roles, permissions
    $operations[] = array('orgright_client_create_roles', array());
    $operations[] = array('orgright_client_create_permissions', array());
    $operations[] = array('orgright_client_create_users', array());
    $operations[] = array('orgright_client_create_forums', array());
    $operations[] = array('orgright_client_create_actions', array());
    // Create initial content - a default public page and a welcome page
    $operations[] = array('orgright_client_create_front_pages', array());
    $operations[] = array('orgright_client_create_error_pages', array());
    $operations[] = array('orgright_client_config_vars', array());

    // Build the batch process
    $batch = array(
      'operations' => $operations,
      'title' => st('Configuring orgRight Client site'),
      'error_message' => st('An error occurred. Please try reinstalling again.'),
      'finished' => 'orgright_client_cleanup',
    );

    // Start the batch
    variable_set('install_task', 'install-orgright-batch');
    batch_set($batch);
    batch_process($url, $url);
  }

  // Persist the page while batch executes
  if ($task == 'install-orgright-batch') {
    include_once 'includes/batch.inc';
    $output = _batch_page();
  }

  return $output;
}

/**
 * Perform initial site installation activities
 *
 * @param $url
 * @return string
 */
function orgright_client_profile_task_profile($url) {
  // Insert default user-defined node types into the database.
  $types = array(
    array(
      'type' => 'page',
      'name' => st('Page'),
      'module' => 'node',
      'description' => st("A <em>page</em>, similar in form to a <em>story</em>, is a simple method for creating and displaying information that rarely changes, such as an \"About us\" section of a website. By default, a <em>page</em> entry does not allow visitor comments and is not featured on the site's initial home page."),
      'custom' => TRUE,
      'modified' => TRUE,
      'locked' => FALSE,
      'help' => '',
      'min_word_count' => '',
    ),
    array(
      'type' => 'story',
      'name' => st('Story'),
      'module' => 'node',
      'description' => st("A <em>story</em>, similar in form to a <em>page</em>, is ideal for creating and displaying content that informs or engages website visitors. Press releases, site announcements, and informal blog-like entries may all be created with a <em>story</em> entry. By default, a <em>story</em> entry is automatically featured on the site's initial home page, and provides the ability to post comments."),
      'custom' => TRUE,
      'modified' => TRUE,
      'locked' => FALSE,
      'help' => '',
      'min_word_count' => '',
    ),
  );

  foreach ($types as $type) {
    $type = (object)_node_type_set_defaults($type);
    node_type_save($type);
  }

  // Update the table of node types
  node_types_rebuild();

  // Default page to not be promoted and have comments disabled.
  variable_set('node_options_page', array('status'));
  variable_set('comment_page', COMMENT_NODE_DISABLED);

  // Don't display date and author information for page nodes by default.
  $theme_settings = variable_get('theme_settings', array());
  $theme_settings['toggle_node_info_page'] = FALSE;
  variable_set('theme_settings', $theme_settings);

  // Update the menu router information.
  menu_rebuild();

  return st('Standard site configuration completed');
}

/**
 * Create necessary directories
 */
function orgright_client_build_directories() {
  $dirs = array('ctools', 'ctools/css', 'pictures', 'imagecache', 'css', 'js');

  foreach ($dirs as $dir) {
    $dir = file_directory_path() . '/' . $dir;
    file_check_directory($dir, TRUE);
  }
}

/**
 * Configure taxonomy
 *
 * Add and configure vocabularies
 */
function orgright_client_config_taxonomy() {
  // Add free-tagging vocabulary for content
  $vocab = array(
    'name' => st('Tags'),
    'description' => st('Free-tagging vocabulary for all content items'),
    'multiple' => '0',
    'required' => '0',
    'hierarchy' => '0',
    'relations' => '1',
    'tags' => '1',
    'module' => 'taxonomy',
  );
  taxonomy_save_vocabulary($vocab);

  // Store the vocabulary id
  variable_set('orgright_client_tags_vid', $vocab['vid']);
}

/**
 * Configure blocks
 */
function orgright_client_config_blocks() {
  $theme = variable_get('theme_default', 'garland');

  // Build the blocks DB table
  _block_rehash($theme);
  _block_rehash('garland');

  // Now, disable all DB blocks
  db_query("UPDATE {blocks} SET status = 0, region = '', weight = 0");

  // Specify the blocks to be configured
  // arguments are: region, weight, module, delta, theme
  $blocks = array();
  // TODO remove devel module
  // For the default user theme
  if ($theme == 'orgrightB') {
    $blocks[$theme . '-user-login'] = array('sidebar_second', '1', 'user', '0', $theme);
    $blocks[$theme . '-search-form'] = array('sidebar_second', '2', 'search', '0', $theme);
    $blocks[$theme . '-navigation'] = array('sidebar_second', '3', 'user', '1', $theme);
    $blocks[$theme . '-devel'] = array('page_closure', '1', 'menu', 'devel', $theme);
  }
  // For the admin theme
  $blocks['garland-user-login'] = array('right', '1', 'user', '0', 'garland');
  $blocks['garland-search-form'] = array('right', '2', 'search', '0', 'garland');
  $blocks['garland-navigation'] = array('right', '3', 'user', '1', 'garland');
  $blocks['garland-devel'] = array('footer', '1', 'menu', 'devel', 'garland');

  foreach ($blocks as $name => $block) {
    //variable_set('block_'.str_replace('-','_',$name), $block);
    db_query("UPDATE {blocks} SET status = 1, region = '%s', weight = %d WHERE module = '%s' AND delta = '%s' AND theme = '%s'", $block);
  }
}

/**
 * Configure menus
 */
function orgright_client_config_menus() {
  // Navigation menu - entry for site "Home"
  $item = array();
  $item['menu_name'] = 'navigation';
  $item['plid'] = 0;
  $item['link_path'] = '<front>';
  $item['link_title'] = st('Home');
  $item['options'] = array("attributes" => array("title" => "Display the front page of this site"));
  $item['module'] = 'menu';
  $item['hidden'] = 0;
  $item['weight'] = "0";
  menu_link_save($item);

  // Enable the Forums menu item
  db_query("UPDATE {menu_links} SET hidden = 0 WHERE menu_name = 'navigation' AND plid = 0 AND link_title = 'Forums'");

  // Set the weighting of menu items in the navigation menu
  $sql = "UPDATE {menu_links} SET weight = %d, customized = 1 WHERE menu_name = 'navigation' AND plid = 0 AND link_title = '%s'";
  db_query($sql, 1, 'Forums');
  db_query($sql, 2, 'Search');
  db_query($sql, 3, 'Create content');
  db_query($sql, 4, 'Public pages');
  db_query($sql, 5, 'Compose tips');
  db_query($sql, 6, 'Administer');
  db_query($sql, 7, 'Helpdesk');
  db_query($sql, 8, 'My account');
  db_query($sql, 9, 'Log out');

  // Hide the orgRight settings entry in the orgright menu
  db_query("UPDATE {menu_links} SET hidden = 1, customized = 1 WHERE menu_name = 'orgright' AND plid = 0 AND link_title = 'orgRight settings'");

  // Adjust the menu settings
  variable_set('menu_default_node_menu', 'navigation');
  variable_set('menu_primary_links_source', 'orgright');
  variable_set('menu_secondary_links_source', 'orgright');
}

/**
 * Configure input filters
 */
function orgright_client_config_filters() {
  // Force filter format and filter IDs
  $filter_formats = "UPDATE {filter_formats} SET format = %d WHERE name = '%s'";
  $filters_sql = "UPDATE {filters} f INNER JOIN {filter_formats} ff ON f.format = ff.format SET f.format = %d WHERE ff.name = '%s'";
  $filters_weight = "UPDATE {filters} SET weight = %d WHERE format = %d AND module = '%s' AND delta = %d";

  // Filtered HTML
  db_query($filters_sql, 1, 'Filtered HTML');
  db_query($filter_formats, 1, 'Filtered HTML');
  // Add the enhanced URL filter to this format
  $filter = new stdClass;
  $filter->format = 1;
  $filter->module = 'orgright';
  $filter->delta = 0;
  $filter->weight = 2;
  drupal_write_record('filters', $filter);
  // set the weighting of the other filters
  db_query($filters_weight, 1, 1, 'filter', 2);
  db_query($filters_weight, 5, 1, 'filter', 0);
  db_query($filters_weight, 6, 1, 'filter', 1);
  db_query($filters_weight, 9, 1, 'filter', 3);
  // Let specified roles use Filtered HTML
  db_query("UPDATE {filter_formats} SET roles = ',3,4,5,6,7,8,' WHERE name = 'Filtered HTML'");

  // Full HTML
  db_query($filters_sql, 2, 'Full HTML');
  db_query($filter_formats, 2, 'Full HTML');
  // Add the enhanced URL filter to this format
  $filter = new stdClass;
  $filter->format = 2;
  $filter->module = 'orgright';
  $filter->delta = 0;
  $filter->weight = 2;
  drupal_write_record('filters', $filter);
  // set the weighting of the other filters
  db_query($filters_weight, 1, 2, 'filter', 2);
  db_query($filters_weight, 6, 2, 'filter', 1);
  db_query($filters_weight, 9, 2, 'filter', 3);
  // specify the allowed html tags for this format
  variable_set('allowed_html_1', '<a> <em> <strong> <cite> <code> <ul> <ol> <li> <dl> <dt> <dd> <p>');
  // Let administrator and site manager roles use Full HTML
  db_query("UPDATE {filter_formats} SET roles = ',3,4,' WHERE name = 'Full HTML'");

  // Create a "links-only" filter format available to all authenticated users
  $format = new stdClass;
  $format->format = 3;
  $format->name = st('Links only');
  $format->roles = ',2,';
  $format->cache = 1;
  drupal_write_record('filter_formats', $format);

  // Add filters to the format
  // HTML filter with no tags allowed
  $filter = new stdClass;
  $filter->format = 3;
  $filter->module = 'filter';
  $filter->delta = 0;
  $filter->weight = 1;
  drupal_write_record('filters', $filter);
  // URL Filter
  $filter = new stdClass;
  $filter->format = 3;
  $filter->module = 'filter';
  $filter->delta = 2;
  $filter->weight = 2;
  drupal_write_record('filters', $filter);
  // Enhanced URL filter
  $filter = new stdClass;
  $filter->format = 3;
  $filter->module = 'orgright';
  $filter->delta = 0;
  $filter->weight = 3;
  drupal_write_record('filters', $filter);
  // Line break converter
  $filter = new stdClass;
  $filter->format = 3;
  $filter->module = 'filter';
  $filter->delta = 1;
  $filter->weight = 4;
  drupal_write_record('filters', $filter);

  // Adjust settings for the filter
  variable_set('filter_url_length_3', 72);
  variable_set('filter_html_3', 2);
  variable_set('filter_html_help_3', 0);
  variable_set('allowed_html_3', '');
  // Specify this as the default filter format
  variable_set('filter_default_format', '3');

  /*****
  // Remove the HTML filter from Filtered HTML
  db_query("DELETE FROM {filters} WHERE format = 1 AND module = 'filter' AND delta = 0");

  // Add WYSIWYG filter to Filtered HTML
  $filter = new stdClass;
  $filter->format = 1;
  $filter->module = 'wysiwyg_filter';
  $filter->delta = 0;
  $filter->weight = -8;
  drupal_write_record('filters', $filter);

  // Adjust the weight of the HTML corrector for Filtered HTML
  db_query("UPDATE {filters} SET weight = -7 WHERE module = 'filter' AND delta = 3");
   *******/

}

/**
 * Configure password policy
 */
function orgright_client_config_password() {
  // Add the password policy
  $policy = new stdClass;
  $policy->pid = 1;
  $policy->name = st('Constraints');
  $policy->description = st('Default list of password constraints');
  $policy->enabled = 1;
  $policy->policy = array(
    'alphanumeric' => 7, // Contain at least 7 alphanumeric chars
    'username' => 1, // Must not equal the username
    'length' => 7, // Must be longer than 7 chars
    'punctuation' => 0, // Punctuation isn't required
  );
  $policy->policy = serialize($policy->policy);
  $policy->created = time();
  drupal_write_record('password_policy', $policy);

  // Attach the policy to the authenticated user role
  $policy_role = new stdClass;
  $policy_role->rid = 2;
  $policy_role->pid = $policy->pid;
  drupal_write_record('password_policy_role', $policy_role);

  // Make the restrictions visible when changing your password
  variable_set('password_policy_show_restrictions', 1);
}

/**
 * Configure wysiwyg
 */
function orgright_client_config_wysiwyg() {
  // Load external file containing editor settings
  orgright_client_include('editor');

  $settings = orgright_client_editor_settings();

  // Add settings for 'Filtered HTML'
  $item = new stdClass;
  $item->format = 1;
  $item->editor = CLIENT_EDITOR;
  $item->settings = serialize($settings['Filtered HTML']);
  drupal_write_record('wysiwyg', $item);

  // Add settings for 'Full HTML'
  $item = new stdClass;
  $item->format = 2;
  $item->editor = CLIENT_EDITOR;
  $item->settings = serialize($settings['Full HTML']);
  drupal_write_record('wysiwyg', $item);
}

/**
 * Configure user_relationships
 */
function orgright_client_config_ur() {
  // Add initial relationship type 'Friend'
  $relationship = new stdClass;
  $relationship->name = st('follower');
  $relationship->plural_name = st('users you follow');
  $relationship->requires_approval = 0;
  $relationship->expires_val = 0;
  $relationship->is_oneway = 1;
  $relationship->is_reciprocal = 1;
  $type = 'insert';

  // Save relationship
  drupal_write_record('user_relationship_types', $relationship);

  // Alert other modules about the new relationship
  $hook = 'user_relationships_type';
  foreach (module_implements($hook) as $module) {
    $function = $module . '_' . $hook;
    $function($type, $relationship);
  }
}

/**
 * Configure views
 *
 * Disable unneeded views to avoid confusion
 * This is helpful because we've cloned many OG views
 */
function orgright_client_config_views() {
  // First fetch any disabled views, just in case
  $disabled = variable_get('views_defaults', array());

  // Now add the views we want to disable
  $disabled['og'] = TRUE;
  $disabled['og_members_block'] = TRUE;
  $disabled['og_my'] = TRUE;

  // Disable views and update the cache
  variable_set('views_defaults', $disabled);
  views_invalidate_cache();
}

/**
 * Configure default images
 *
 * The group and profile default images need to be processed by ImageCache
 * and stored in the files directory
 */
function orgright_client_config_images() {
  // Copy default user image to files directory
  $user_image = 'profiles/orgright_client/images/default-user.png';
  file_copy($user_image, 0, FILE_EXISTS_REPLACE); // Defaults to files directory

  // Copy default group image to files directory
  $group_image = 'profiles/orgright_client/images/default-group.png';
  file_copy($group_image, 0, FILE_EXISTS_REPLACE); // Defaults to files directory

  // Process default user image through ImageCache
  $preset = imagecache_preset_by_name('profile_pictures');
  imagecache_build_derivative(
    $preset['actions'],
    $user_image,
    file_directory_path() . '/imagecache/profile_pictures/default-user.png'
  );

  // Process default user image thumbnail through ImageCache
  $preset = imagecache_preset_by_name('user_picture_meta');
  imagecache_build_derivative(
    $preset['actions'],
    $user_image,
    file_directory_path() . '/imagecache/user_picture_meta/default-user.png'
  );

  // Set user image as the default
  variable_set('user_picture_default', 'default-user.png');

  // Process default group image through ImageCache
  $preset = imagecache_preset_by_name('group_images');
  imagecache_build_derivative(
    $preset['actions'],
    $group_image,
    file_directory_path() . '/imagecache/group_images/imagefield_default_images/default-group.png'
  );

  // Process default group image thumbnail through ImageCache
  $preset = imagecache_preset_by_name('group_images_thumb');
  imagecache_build_derivative(
    $preset['actions'],
    $group_image,
    file_directory_path() . '/imagecache/group_images_thumb/imagefield_default_images/default-group.png'
  );

  // Simulate that we've uploaded the group image
  $file = new stdClass;
  $file->uid = 1;
  $file->filename = 'default-group.png';
  $file->filepath = file_directory_path() . '/' . $file->filename;
  $file->filemime = 'image/png';
  $file->filesize = filesize($group_image);
  $file->status = 1;
  $file->timestamp = time();
  drupal_write_record('files', $file);
}

/**
 * Configure variables
 *
 * These should be set but not enforced by Strongarm
 */
function orgright_client_config_vars() {
  // Set user terms to use the "tags" vocabulary we created
  $vid = variable_get('orgright_client_tags_vid', 1);
  variable_set('user_terms_vocabs', array($vid => $vid));

  // Show large amount of tags on tag cloud page
  variable_set('tagadelic_page_amount', 500);

  // Specify performance settings
  variable_set('block_cache', "0");
  variable_set('cache', "1");
  variable_set('cache_lifetime', "0");
  variable_set('page_compression', "1");
  variable_set('preprocess_css', "1");
  variable_set('preprocess_js', "1");

  // Keep errors in the log and off the screen
  variable_set('error_level', 0);

  // Don't restrict user profile image upload size
  variable_set('user_picture_file_size', '');

  // Only report security updates
  variable_set('update_notification_threshold', 'security');

  // Set date/timezone variables
  variable_set('date_api_use_iso8601', FALSE);
  variable_set('date_api_version', "5.2");
  variable_set('date_default_timezone_name', "Pacific/Auckland");
  variable_set('date_first_day', "1");
  variable_set('date_format_long', "l, j F Y - g:ia");
  variable_set('date_format_medium', "D, d/m/Y - g:ia");
  variable_set('date_format_short', "d/m/Y - g:ia");

  // Set Print, PDF and mail settings
  variable_set('print_comments', 0);
  variable_set('print_css', "");
  variable_set('print_footer_options', "1");
  variable_set('print_footer_user', "");
  variable_set('print_html_book_link', "1");
  variable_set('print_html_link_class', "print-page");
  variable_set('print_html_link_pos', array('link' => 'link', 'corner' => 0, 'block' => 'block', 'help' => 'help'));
  variable_set('print_html_link_teaser', 0);
  variable_set('print_html_link_use_alias', 0);
  variable_set('print_html_new_window', 1);
  variable_set('print_html_node_link_pages', "");
  variable_set('print_html_node_link_visibility', "0");
  variable_set('print_html_sendtoprinter', 0);
  variable_set('print_html_show_link', "3");
  variable_set('print_html_sys_link_pages', "admin\nadmin/*");
  variable_set('print_html_sys_link_visibility', "0");
  variable_set('print_html_windowclose', 1);
  variable_set('print_logo_options', "0");
  variable_set('print_logo_upload', "");
  variable_set('print_logo_url', "");
  variable_set('print_mail_book_link', "1");
  variable_set('print_mail_hourly_threshold', "10");
  variable_set('print_mail_link_class', "print-mail");
  variable_set('print_mail_link_pos', array('link' => 'link', 'corner' => 0, 'block' => 'block', 'help' => 'help'));
  variable_set('print_mail_link_teaser', 0);
  variable_set('print_mail_link_use_alias', 0);
  variable_set('print_mail_node_link_pages', "");
  variable_set('print_mail_node_link_visibility', "0");
  variable_set('print_mail_send_option_default', "sendpage");
  variable_set('print_mail_show_link', "3");
  variable_set('print_mail_sys_link_pages', "admin\nadmin/*");
  variable_set('print_mail_sys_link_visibility', "0");
  variable_set('print_mail_teaser_choice', 1);
  variable_set('print_mail_teaser_default', 1);
  variable_set('print_newwindow', "1");
  variable_set('print_pdf_autoconfig', 1);
  variable_set('print_pdf_book_link', "1");
  variable_set('print_pdf_content_disposition', "2");
  variable_set('print_pdf_dompdf_unicode', 0);
  variable_set('print_pdf_filename', "[current-page-title] ([site-date-dd][site-date-mon][site-date-yy])");
  variable_set('print_pdf_font_family', "helvetica");
  variable_set('print_pdf_font_size', "10");
  variable_set('print_pdf_link_class', "print-pdf");
  variable_set('print_pdf_link_pos', array('link' => 'link', 'corner' => 0, 'block' => 'block', 'help' => 'help'));
  variable_set('print_pdf_link_teaser', 0);
  variable_set('print_pdf_link_use_alias', 0);
  variable_set('print_pdf_node_link_pages', "");
  variable_set('print_pdf_node_link_visibility', "0");
  variable_set('print_pdf_page_orientation', "portrait");
  variable_set('print_pdf_paper_size', "A4");
  variable_set('print_pdf_pdf_tool', "profiles/orgright_client/modules/print/lib/dompdf/dompdf_config.inc.php");
  variable_set('print_pdf_show_link', "3");
  variable_set('print_pdf_sys_link_pages', "admin\nadmin/*");
  variable_set('print_pdf_sys_link_visibility', "0");
  variable_set('print_pdf_wkhtmltopdf_options', "--footer-font-size 7 --footer-right '[page]'");
  variable_set('print_robots_noarchive', 0);
  variable_set('print_robots_nofollow', 1);
  variable_set('print_robots_noindex', 1);
  variable_set('print_sourceurl_date', 0);
  variable_set('print_sourceurl_enabled', 1);
  variable_set('print_sourceurl_forcenode', 0);
  variable_set('print_urls', 1);
  variable_set('print_urls_anchors', 0);

  // Set the site information
  variable_set('site_footer', '&copy;2012 <a href="http://orgright.com" target="_blank">orgRight Limited</a> - All rights reserved');
  //variable_set('site_frontpage', 'node');
  //variable_set('site_mail', 'demo@orgright.co.nz');
  //variable_set('site_mission', '');
  //variable_set('site_name', '');
  //variable_set('site_offline_message', 'Your orgRight site is currently off-line for maintenance. Please come back later');
  //variable_set('site_slogan', '');

  // Set the theme default valuess
  $theme_settings = array(
    // Display items
    'toggle_logo' => 1,
    'toggle_name' => 1,
    'toggle_slogan' => 1,
    'toggle_mission' => 0,
    'toggle_node_user_picture' => 0,
    'toggle_comment_user_picture' => 0,
    'toggle_search' => 0,
    'toggle_favicon' => 1,
    'toggle_primary_links' => 1,
    'toggle_secondary_links' => 1,
    // Show "post" information on node types
    //'toggle_node_info_agenda_item' => 1,
    //'toggle_node_info_appointment' => 1,
    //'toggle_node_info_assignment' => 1,
    //'toggle_node_info_committee' => 1,
    //'toggle_node_info_role' => 1,
    //'toggle_node_info_document' => 1,
    //'toggle_node_info_drawer' => 1,
    //'toggle_node_info_folder' => 1,
    //'toggle_node_info_forum' => 1,
    //'toggle_node_info_meeting' => 1,
    //'toggle_node_info_mtg_item' => 1,
    //'toggle_node_info_member' => 1,
    'toggle_node_info_page' => 0,
    //'toggle_node_info_project' => 1,
    //'toggle_node_info_projactivity' => 1,
    //'toggle_node_info_projreport' => 1,
    'toggle_node_info_public' => 0,
    //'toggle_node_info_story' => 1,
    //'toggle_node_info_subscription' => 1,
    //'toggle_node_info_subpayment' => 1,
    // Logo and Favicon info
    'default_logo' => 1,
    //'logo_path' => '',
    //'logo_upload' => '',
    'default_favicon' => 1,
    //'favicon_path' => '',
    //'favicon_upload' => '',
  );
  variable_set('theme_settings', $theme_settings);

  // Specify the admin theme
  variable_set('admin_theme', ORGRIGHT_CLIENT_ADMIN_THEME);

  // Set the default Filter format for orgRight fields with URL info
  variable_set('orgright_field_filter', '3'); // Links only

  // Set the site search parameters
  variable_set('node_rank_relevance', "7");
  variable_set('node_rank_recent', "5");
  variable_set('node_rank_comments', "3");
  //variable_set('search_cron_limit', '100');
  //variable_set('overlap_cjk', 1);

  // Specfiy orgRight module settings
  variable_set('filecabinet_doclink_nodetypes', array(
    'agenda_item',
    'appointment',
    'assignment',
    'committee',
    'role',
    'meeting',
    'mtg_item',
    'member',
    'project',
    'projactivity',
    'projreport',
    'subscription',
    'subpayment',
  ));

  // Specify the file system settings
  $domain = orgright_client_get_domain();
  $mode = FILE_CREATE_DIRECTORY && FILE_MODIFY_PERMISSIONS;
  $file_dir = 'sites/' . $domain . '/files';
  if (file_check_directory($file_dir, $mode)) {
    variable_set('file_directory_path', $file_dir);
    $temp_dir = $file_dir . '/tmp';
  }
  else {
    $temp_dir = '/tmp/orgright/' . $domain;
  }
  if (file_check_directory($temp_dir, $mode)) {
    variable_set('file_directory_temp', $temp_dir);
  }

  // Specify file upload size defaults (sizes in MB)
  variable_set('upload_uploadsize_default', "10");
  variable_set('upload_usersize_default', "10000");

  // Specify the content types that can have upload files
  variable_set('upload_page', "1");
  variable_set('upload_story', "1");

  // Specify the content types that can have comments
  $comment_types = array(
    'story',
    'member',
    'subscription',
    'subpayment',
    'committee',
    'role',
    'appointment',
    'assignment',
    'meeting',
    'agenda_item',
    'mtg_item',
    'project',
    'projactivity',
    'projreport',
    'drawer',
    'folder',
    'document',
  );
  foreach ($comment_types as $type) {
    variable_set('comment_' . $type, "2"); // Read/write
    variable_set('comment_default_mode_' . $type, "2"); // Flat list - expanded
    variable_set('comment_default_order' . $type, "2"); // Newest first
    variable_set('comment_default_per_page' . $type, "12"); // Number of comments per page
    variable_set('comment_controls_' . $type, "3"); // Do not display
    variable_set('comment_subject_field_' . $type, "0"); // Disabled
    variable_set('comment_preview_' . $type, "0"); // Preview comment is optional
    variable_set('comment_form_location_' . $type, "0"); // Display comment submission form on separate page
  }

}

function orgright_client_config_backup_migrate() {
  // Set up a daily backup profile and schedule
  require_once './' . drupal_get_path('module', 'backup_migrate') . '/backup_migrate.module';
  require_once './' . drupal_get_path('module', 'backup_migrate') . '/includes/crud.inc';
  require_once './' . drupal_get_path('module', 'backup_migrate') . '/includes/profiles.inc';
  require_once './' . drupal_get_path('module', 'backup_migrate') . '/includes/files.inc';
  require_once './' . drupal_get_path('module', 'backup_migrate') . '/includes/schedules.inc';

  $settings = array(
    'profile_id' => 'daily_backup',
    'filename' => 'daily-backup',
    'append_timestamp' => TRUE,
    'timestamp_format' => 'Y-m-d',
    'filters' => array(
      'compression' => "none",
      'exclude_tables' => array(),
      'nodata_tables' => array(),
    ),
    'name' => t('Daily Backup'),
  );
  $profile = backup_migrate_crud_create_item('profile', $settings);
  $profile->save();

  $schedule = array(
    'name' => t('Daily Backup'),
    'profile_id' => 'daily_backup',
    'enabled' => 1,
    'destination_id' => 'scheduled',
    'period' => 86400,
    'keep' => '3',
  );
  $schedule = backup_migrate_crud_create_item('schedule', $schedule);
  $schedule->save();
}

function orgright_client_create_users() {
  // Create the standard users for an organisation site
  // parameters are: name, password, email, roles, status (0=blocked)
  $user =  install_add_user('orgright', 'soFar-soG00d', 'user3@orgright.com', array(), 1);
  // now add dummy users up to O_MAX_PROTECTED_USER
  for ($uid = $user->uid + 1; $uid <= O_MAX_PROTECTED_USER; $uid++) {
    install_add_user('user'.$uid, 'user'.$uid.'~org+Z', 'user'.$uid.'@orgright.com', array(), 0);
  }

}

function orgright_client_create_roles() {
  // Create the standard roles for an organisation site
  $roles = array(
    array('rid' => 3, 'name' => 'administrator',),
    array('rid' => 4, 'name' => 'site manager',),
    array('rid' => 5, 'name' => 'president',),
    array('rid' => 6, 'name' => 'secretary',),
    array('rid' => 7, 'name' => 'treasurer',),
    array('rid' => 8, 'name' => 'membership',),
  );
  foreach ($roles as $role) {
    $record = new stdClass();
    $record->rid = $role['rid'];
    $record->name = $role['name'];
    drupal_write_record('role', $record);
  }
}

function orgright_client_create_permissions() {
  // Create the standard permissions for each role
  // Role 3: administrator gets all permissions, other roles get only
  // the permissions for which they are listed in the value array.
  $permissions = array(
    // orgRight membership module
    'view member' => array(2, 4),
    'create member' => array(4, 6, 8),
    'update member' => array(4, 6, 8),
    'delete member' => array(4, 6, 8),
    'view subscription' => array(2, 4),
    'create subscription' => array(4, 7, 8),
    'update subscription' => array(4, 7, 8),
    'delete subscription' => array(4, 7, 8),
    'view subpayment' => array(2, 4),
    'create subpayment' => array(4, 7, 8),
    'update subpayment' => array(4, 7, 8),
    'delete subpayment' => array(4, 7, 8),

    // orgRight committee module
    'view committee' => array(2, 4),
    'create committee' => array(4, 5, 6),
    'update committee' => array(4, 5, 6),
    'delete committee' => array(4, 5, 6),
    'view role' => array(2, 4),
    'create role' => array(4, 5, 6),
    'update role' => array(4, 5, 6),
    'delete role' => array(4, 5, 6),
    'view appointment' => array(2, 4),
    'create appointment' => array(4, 5, 6),
    'update appointment' => array(4, 5, 6),
    'delete appointment' => array(4, 5, 6),
    'view assignment' => array(2, 4),
    'create assignment' => array(4, 5, 6),
    'update assignment' => array(4, 5, 6),
    'delete assignment' => array(4, 5, 6),

    // orgRight meeting module
    'view meeting' => array(2, 4),
    'create meeting' => array(4, 5, 6),
    'update meeting' => array(4, 5, 6),
    'delete meeting' => array(4, 5, 6),
    'view agenda item' => array(2, 4),
    'create agenda item' => array(4, 5, 6),
    'update agenda item' => array(4, 5, 6),
    'delete agenda item' => array(4, 5, 6),
    'view meeting item' => array(2, 4),
    'create meeting item' => array(4, 5, 6),
    'delete meeting item' => array(4, 5, 6),
    'update meeting item' => array(4, 5, 6),

    // orgRight project module
    'view project' => array(2, 4),
    'create project' => array(4, 5, 6),
    'update project' => array(4, 5, 6),
    'delete project' => array(4, 5, 6),
    'view project activity' => array(2, 4),
    'create project activity' => array(4, 5, 6),
    'update project activity' => array(4, 5, 6),
    'delete project activity' => array(4, 5, 6),
    'create project activity link' => array(4, 5, 6),
    'update project activity link' => array(4, 5, 6),
    'delete project activity link' => array(4, 5, 6),
    'view project report' => array(2, 4),
    'create project report' => array(4, 5, 6),
    'update project report' => array(4, 5, 6),
    'delete project report' => array(4, 5, 6),

    // orgRight filecabinet module
    'view drawer' => array(2, 4),
    'create drawer' => array(4, 5, 6),
    'update drawer' => array(4, 5, 6),
    'delete drawer' => array(4, 5, 6),
    'view folder' => array(2, 4),
    'create folder' => array(4, 5, 6),
    'update folder' => array(4, 5, 6),
    'delete folder' => array(4, 5, 6),
    'view document' => array(2, 4),
    'create document' => array(4, 5, 6, 7, 8),
    'update document' => array(4, 5, 6, 7, 8),
    'delete document' => array(4, 5, 6),
    'view file' => array(2, 4),
    'upload file' => array(4, 5, 6, 7, 8),
    'replace file' => array(),
    'delete file' => array(),

    // orgRight helpservice module
    'view orgright help index' => array(2, 4),
    'view orgright help popup' => array(2, 4),
    'view orgright help topic' => array(2, 4),

    // orgRight administration
    'view public page' => array(1),
    'create public page' => array(),
    'update public page' => array(),
    'delete public page' => array(),
    'use orgRight helpdesk' => array(2, 4),
    'administer orgRight modules' => array(),

    // Administration menu
    'access administration menu' => array(),
    'display drupal links' => array(),

    // Backup and migrate module
    'perform backup' => array(),
    'access backup files' => array(),
    'delete backup files' => array(),

    // Comment module
    'access comments' => array(4),
    'administer comments' => array(),
    'post comments' => array(2, 4),
    'post comments without approval' => array(2, 4),

    // Forum module
    'administer forums' => array(),
    'access forums' => array(2, 4), // This permission is defined by orgRight
    'create forum topics' => array(2, 4),
    'delete any forum topic' => array(),
    'delete own forum topics' => array(4, 5, 6, 7, 8),
    'edit any forum topic' => array(),
    'edit own forum topics' => array(4, 5, 6, 7, 8),

    // Feedback module
    'access feedback form' => array(4,),
    'view feedback messages' => array(),

    // menu module
    'administer menu' => array(),

    // node module
    'access content' => array(1, 2),
    'administer nodes' => array(),
    'create page content' => array(4, 5, 6, 7, 8),
    'create story content' => array(4, 5, 6, 7, 8),
    'delete any page content' => array(),
    'delete any story content' => array(),
    'delete own page content' => array(4, 5, 6, 7, 8),
    'delete own story content' => array(4, 5, 6, 7, 8),
    'delete revisions' => array(),
    'edit any page content' => array(),
    'edit any story content' => array(),
    'edit own page content' => array(4, 5, 6, 7, 8),
    'edit own story content' => array(4, 5, 6, 7, 8),
    'revert revisions' => array(),
    'view revisions' => array(),

    // path module
    'create url aliases' => array(),
    'administer url aliases' => array(),

    // print module
    'access print' => array(2, 4),
    'access send by email' => array(2, 4),
    'access PDF version' => array(2, 4),
    'administer print' => array(),

    // rules module
    'administer rules' => array(),

    // search module
    'administer search' => array(),
    'search content' => array(2, 4),
    'use advanced search' => array(2, 4),
    'view advanced help index' => array(2, 4),
    'view advanced help popup' => array(2, 4),
    'view advanced help topic' => array(2, 4),

    // system module
    'access administration pages' => array(),
    'administer actions' => array(),

    // taxonomy module
    'administer taxonomy' => array(),

    // upload module
    'upload files' => array(4, 5, 6, 7, 8),
    'view uploaded files' => array(2, 4),

    // user module
    'access user profiles' => array(),
    'administer permissions' => array(),
    'administer users' => array(),
    'change own username' => array(),

    // views module
    'access all views' => array(2, 4),
    'administer views' => array(),
  );

  $role_permissions = array();
  foreach ($permissions as $permission => $roles) {
    // Ensure role 3: administrator gets this permission
    $roles[] = 3;

    // Give all specified roles specified this permission
    foreach ($roles as $rid) {
      if (isset($role_permissions[$rid])) {
        $role_permissions[$rid] .= ', ' . $permission;
      }
      else {
        $role_permissions[$rid] = $permission;
      }
    }
  }

  foreach ($role_permissions as $rid => $permissions) {
    // find an existing record if any for this rid
    $pid = db_result(db_query("SELECT pid FROM {permission} WHERE rid = %d", $rid));
    $record = new stdClass();
    $key = NULL;
    if ($pid) {
      $record->pid = $pid;
      $key = 'pid';
    }
    $record->rid = $rid;
    $record->perm = $permissions;
    $record->tid = 0;
    drupal_write_record('permission', $record, $key);
  }
}

/**
 * Create any initial forums
 */
function orgright_client_create_forums() {
  // refer to http://www.unibia.com/unibianet/drupal/how-create-drupal-forums-and-containers-programmatically
  require_once 'modules/forum/forum.admin.inc';
  $forum_vid = 1; // Forum vocabulary is vocab 1
  // Create a container
  $forum_container_fields = array();
  $forum_container_fields['values']['name'] = st('Miscellaneous');
  $forum_container_fields['values']['description'] = st('This container is for forums of a general nature');
  $forum_container_fields['values']['parent'][0] = 0;
  $forum_container_fields['values']['weight'] = 0;
  $forum_container_fields['values']['vid'] = $forum_vid;
  $container = forum_form_container($forum_container_fields);
  $container['form_id']['#value'] = 'forum_form_container'; // save the form id correctly
  forum_form_submit($container, $forum_container_fields);

  // Create an initial forum
  $containerID = taxonomy_get_term_by_name(st('Miscellaneous'));
  $forum_topic_fields = array();
  $forum_topic_fields['values']['name'] = st('General');
  $forum_topic_fields['values']['description'] = st('This forum is for discussions of a general nature about the organisation');
  $forum_topic_fields['values']['parent'][0] = $containerID[0]->tid;
  $forum_topic_fields['values']['weight'] = 0;
  $forum_topic_fields['values']['vid'] = $forum_vid;
  $forum = forum_form_forum($forum_topic_fields);
  forum_form_submit($forum, $forum_topic_fields);

}

/**
 * Create any initial non-default actions
 */
function orgright_client_create_actions() {
  // create an action to change the author of a post
  $action = new stdClass;
  $action->type = 'node';
  $action->callback = 'node_assign_owner_action';
  $action->parameters = serialize(array('owner_uid' => O_MAX_PROTECTED_USER + 1));
  $action->description = 'Change the author of a post';

  // first get a new aid index then write the action record
  drupal_write_record('actions_aid', $action);
  drupal_write_record('actions', $action);

}

function orgright_client_create_front_pages() {
  // Create the front public access page for anonymous users
  $node = new stdClass;
  $node->type = 'public';
  $node->title = st('Authorised Access Only');
  $node->body = st('<p><strong>Access to this site is only available to authorised persons</strong></p><p>You will need a login and password to use this site. If you do not have a login, you will need to contact an appropriate member of your organisation to receive a login. If you do have a login, but have lost/forgotten your password, then you will need to contact the person in your organisation responsible for administering this site to get a new one issued.</p>');
  $node->teaser = $node->body;
  $node->format = 1;
  $node->uid = 1;
  $node->status = 1;
  $node->promote = 1;
  $node->sticky = 1;
  $node->created = time();
  $node->changed = time();
  $node->comment = 0;
  $node->moderate = 0;
  $node->tnid = 0;
  $node->translate = 0;
  $node->language = '';

  //orgright_client_include('node');
  node_save($node);

  // Create the front "welcome page" for logged in users
  $node = new stdClass;
  $node->type = 'page';
  $node->title = st('Welcome');
  $node->body = st('<p><strong>Welcome to your orgRight site.</strong></p><p>This site is for members of the committee and other individuals involved in managing or running your organisation.  It does not provide features that will allow it to be used as a general access site for communication with members of the public about your organisation.</p>');
  $node->teaser = $node->body;
  $node->format = 1;
  $node->uid = 1;
  $node->status = 1;
  $node->promote = 1;
  $node->sticky = 1;
  $node->created = time();
  $node->changed = time();
  $node->comment = 0;
  $node->moderate = 0;
  $node->tnid = 0;
  $node->translate = 0;
  $node->language = '';

  //orgright_client_include('node');
  node_save($node);

  // Create a URL aliases for this page
  $alias = new stdClass;
  $alias->src = 'node/' . $node->nid;
  $alias->dst = 'welcome';
  drupal_write_record('url_alias', $alias);

}

function orgright_client_create_error_pages() {
  // Create the 403 - Access Denied error page
  $node = new stdClass;
  $node->type = 'page';
  $node->title = st('Access Denied');
  $node->body = st('<h1>Access Denied</h1>You do not have sufficient permissions to access the requested page.</p>');
  $node->teaser = $node->body;
  $node->format = 2;
  $node->uid = 1;
  $node->status = 1;
  $node->promote = 0;
  $node->sticky = 0;
  $node->created = time();
  $node->changed = time();
  $node->comment = 0;
  $node->moderate = 0;
  $node->tnid = 0;
  $node->translate = 0;
  $node->language = '';

  //orgright_client_include('node');
  node_save($node);
  variable_set('site_403', "node/" . $node->nid);

  // Create the 404 - Does not exist error page
  $node = new stdClass;
  $node->type = 'page';
  $node->title = st('Does Not Exist');
  $node->body = st('<h1>Does Not Exist</h1>The page you have requested doe not exist.</p>');
  $node->teaser = $node->body;
  $node->format = 2;
  $node->uid = 1;
  $node->status = 1;
  $node->promote = 0;
  $node->sticky = 0;
  $node->created = time();
  $node->changed = time();
  $node->comment = 0;
  $node->moderate = 0;
  $node->tnid = 0;
  $node->translate = 0;
  $node->language = '';

  //orgright_client_include('node');
  node_save($node);
  variable_set('site_404', "node/" . $node->nid);
}

/**
 * Configure tidy node links
 */
function orgright_client_config_tidy_node_links() {
  $theme = variable_get('theme_default', 'garland');
  if ($theme == 'orgright_client_origins') {
    drupal_install_modules(array('tidy_node_links'));
  }
}

/**
 * Create an initial group with a discussion
 */
function orgright_client_create_group() {
  orgright_client_include('node');

  // Create the group
  $group = _orgright_client_default_group_node();
  node_save($group);

  // Check if discussion nodes were enabled
  if (in_array('orgright_client_discussion', variable_get('orgright_client_selected_features', array()))) {
    // Create the discussion
    $node = _orgright_client_default_discussion_node();
    $node->og_groups = array($group->nid => $group->nid);
    node_save($node);
  }
}

/**
 * Various actions needed to clean up after the installation
 */
function orgright_client_cleanup() {
  // Rebuild node access database - required after OG installation
  node_access_rebuild();

  // Rebuild node types
  node_types_rebuild();

  // Clear drupal message queue for non-warning/errors
  drupal_get_messages('status', TRUE);

  // Clear out caches
  $core = array('cache', 'cache_block', 'cache_filter', 'cache_page');
  $cache_tables = array_merge(module_invoke_all('flush_caches'), $core);
  foreach ($cache_tables as $table) {
    cache_clear_all('*', $table, TRUE);
  }

  // Clear out JS and CSS caches
  drupal_clear_css_cache();
  drupal_clear_js_cache();

  // Some features will need reverting
  $revert = array(
    'orgright_client_core' => array('menu_links'),
    'orgright_client_notifications' => array('variable'),
    'orgright_client_seo' => array('variable'),
    'orgright_client_blog' => array('menu_links', 'user_permission'),
    'orgright_client_event' => array('menu_links'),
    'orgright_client_poll' => array('menu_links'),
    'orgright_client_document' => array('menu_links'),
    'orgright_client_discussion' => array('menu_links'),
    'orgright_client_wiki' => array('menu_links', 'variable'),
    'orgright_client_home' => array('page_manager_pages'),
    'orgright_client_reputation' => array('menu_links'),
    'orgright_client_admin' => array('user_permission'),
    'orgright_client_answers' => array('user_permission'),
  );

  // Make sure we only try to revert features we've enabled
  $enabled = variable_get('orgright_client_selected_features', array('orgright_client_core'));

  foreach ($revert as $feature => $value) {
    if (!in_array($feature, $enabled)) {
      unset($revert[$feature]);
    }
  }
  //features_revert($revert);

  // Say hello to the dog!
  watchdog('orgright', st('Welcome to your new orgRight site!'));

  // Create a test group which contains a node
  //orgright_client_create_group();
  // Rebuild Activity Log templates.
  //activity_log_rebuild_everything();
  // Remove the feature choices
  variable_del('orgright_client_selected_features');

  // Finish the installation
  variable_set('install_task', 'profile-finished');
}

/**
 * Miscellaneous helper functions
 *
 */
function orgright_client_get_domain() {
  // will return the site base url with the sub-directory (if any) stripped
  global $base_url;
  $ix = strpos($base_url, '://') + 3;
  return ($iy = strpos($base_url, '/', $ix)) ? substr($base_url, $ix, $iy - $ix) : substr($base_url, $ix);
}
