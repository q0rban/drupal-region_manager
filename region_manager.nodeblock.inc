<?php
// $Id: region_manager.nodeblock.inc,v 1.2 2010/02/09 00:13:20 q0rban Exp $

/**
 * @file 
 *
 * Holds functions for the nodeblock implementation of Region Manager's block
 * creation API.
 */

/**
 * Implementation of hook_form_alter() on behalf of nodeblock. We need to use
 * the region_manager namespace since nodeblock already implements this hook.
 */
function region_manager_form_alter(&$form, $form_state, $form_id) {
  if (
    function_exists('nodeblock_block') &&
    $form['#id'] == 'node-form' && 
    variable_get('nodeblock_'. $form['type']['#value'], 0) && 
    !$form['nid']['#value'] && // Verifies we are inserting, not updating.
    ($theme_key = $_GET['theme_key']) &&
    ($region = $_GET['region']) && 
    ($path = $_REQUEST['destination'])
  ) {
    $form['region_manager'] = array(
      '#tree' => TRUE,
    );
    $form['region_manager']['theme_key'] = array(
      '#type' => 'hidden',
      '#value' => $theme_key,
    );
    $form['region_manager']['region'] = array(
      '#type' => 'hidden',
      '#value' => $region,
    );
    $form['region_manager']['module'] = array(
      '#type' => 'hidden',
      '#value' => 'nodeblock',
    );
    $form['region_manager']['path'] = array(
      '#type' => 'hidden',
      '#value' => $path,
    );
  }
}

/**
 * Implementation of hook_region_manager_create_links() on behalf of nodeblock.
 *
 * @param $theme_key
 *   A string containing the key of the theme that is being managed.
 * @param $region
 *   A string containing the region being managed.
 */
function nodeblock_region_manager_create_links($theme_key, $region) {
  $links = array();
  $records = region_manager_regions_load($theme_key);

  if ($records[$region]['modules']['nodeblock'] != REGION_MANAGER_ALL_DISABLED) {
    $types = node_get_types();
    foreach ($types as $type) {
      if (nodeblock_type_enabled($type)) {
        if (node_access('create', $type)) {
          $type_url_str = str_replace('_', '-', $type->type);
          $path = "node/add/$type_url_str";
          $links[$path] = array(
            'title' => t('@type @block', array('@type' => $type->name, '@block' => _region_manager_block_name())),
            'path' => $path,
          );
        }
      }
    }
  }

  return $links;
}