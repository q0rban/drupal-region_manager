<?php
// $Id: region_manager.inc,v 1.2 2010/02/09 00:13:20 q0rban Exp $

/**
 * @file
 *   Holds all API functions for region_manager.module
 */

/**
 * Retrieves a set of enabled Region Manager regions for the theme.
 * If the variable has not been set for this particular theme, and 
 * the theme is a sub-theme, it will look to each base theme until 
 * it finds a setting. Returns an empty array if no settings found.
 *
 * @param $theme_key
 *   String name of a theme.
 * @return
 *   An array of regions for the theme.
 */
function region_manager_regions_variable_get($theme_key) {
  $themes = list_themes();
  $theme_info = $themes[$theme_key];

  $regions = variable_get('region_manager_regions_'. $theme_key, array());

  if (empty($regions) && isset($theme_info->info['base theme'])) {
    $regions = region_manager_regions_variable_get($theme_info->info['base theme']);
  }

  return $regions;
}

/**
 * Get a list of available regions from a specified theme.
 *
 * @param $theme_key
 *   The name of a theme.
 * @return
 *   An array of regions in the form $region['name'] = 'description'.
 */
function region_manager_region_list($theme_key) {
  static $region_list = array();

  if (empty($region_list)) {
    $settings = region_manager_regions_variable_get($theme_key);
    if (!empty($settings)) {
      $regions = system_region_list($theme_key);
      $region_list = array_intersect_key($regions, array_filter($settings));
    }
  }

  return $region_list;
}

/**
 * Loader function for regions by theme or rid.
 *
 * @param $theme_key
 *   A string containing the name of the theme.
 * @param $reset
 *   Boolean, if TRUE the cache will be flushed.
 * @return
 *   A single record in array format, or FALSE if none matched the incoming ID.
 */
function region_manager_regions_load($theme_key, $reset = FALSE) {
  static $records = array();

  $cache_key = 'region_manager:'. $theme_key;

  if ($reset) {
    cache_clear_all($cache_key, 'cache');
    $records = array();
  }

  if (!empty($records)) {
    return $records;
  }

  if (($cache = cache_get($cache_key, 'cache')) && is_array($cache->data)) {
    $records = $cache->data;
  }
  else {
    $sql = "SELECT * FROM {region_manager_regions} WHERE theme_key = '%s'";
    $result = db_query($sql, $theme_key);
  
    while ($record = db_fetch_array($result)) {
      $record['modules'] = unserialize($record['modules']);
      $record['blocked_blocks'] = explode(', ', $record['blocked_blocks']);
      $record['storage'] = REGION_MANAGER_STORAGE_NORMAL;

      $records[$record['region']] = $record;
    }

    // Call hook_region_manager_defaults().
    foreach(module_invoke_all('region_manager_defaults', $theme_key) as $region => $record) {
      if ($records[$region]) {
        $records[$region]['storage'] = REGION_MANAGER_STORAGE_OVERRIDE;
      }
      else {
        $record['storage'] = REGION_MANAGER_STORAGE_DEFAULT;
        $records[$region] = $record;
      }
    }
    cache_set($cache_key, $records);
  }

  // If there are no records, look to the base theme and see if it has records.
  if (empty($records)) {  
    $themes = list_themes();
    $theme_info = $themes[$theme_key];

    if (isset($theme_info->info['base theme'])) {
      $records = region_manager_regions_load($theme_info->info['base theme']);
    }
  }

  return $records;
}

/**
 * Public loader function for the full collection of records.
 *
 * @return
 *   An array of all records, keyed by id.
 */
function region_manager_regions_load_all() {
  $sql = 'SELECT * FROM {region_manager_regions} ORDER BY rid ASC';
  $result = db_query($sql);

  $records = array();
  while ($record = db_fetch_array($result)) {
    $record['modules'] = unserialize($record['modules']);
    $record['blocked_blocks'] = explode(', ', $record['blocked_blocks']);
    $record['storage'] = REGION_MANAGER_STORAGE_NORMAL;

    $records[$record['theme_key'] .':'. $record['region']] = $record;
  }

  // Call hook_region_manager_defaults().
  foreach(module_invoke_all('region_manager_defaults') as $theme_key => $theme) {
    foreach($theme as $region => $record) {
      $key = $theme_key .':'. $region;
      if ($records[$key]) {
        $records[$key]['storage'] = REGION_MANAGER_STORAGE_OVERRIDE;
      }
      else {
        $record['storage'] = REGION_MANAGER_STORAGE_DEFAULT;
        $records[$key] = $record;
      }
    }
  }

  return $records;
}

/**
 * Inserts a new record, or updates an existing one.
 *
 * @param $region
 *   A record to be saved.
 * @return
 *   The saved record.
 */
function region_manager_region_save($region) {
  if (isset($region['rid'])) {
    drupal_write_record('region_manager_regions', $region, 'rid');
  }
  else {
    drupal_write_record('region_manager_regions', $region);
  }
  return $region;
}

/**
 * Deletes a record, given its unique ID.
 *
 * @param $rid
 *   An integer containing the ID of a record.
 */
function region_manager_region_delete($rid) {
  $sql = "DELETE FROM {region_manager_regions} WHERE rid = %d";
  db_query($sql, $rid);
}

/**
 * Loads all available blocks organized by region.
 */
function region_manager_blocks_load_all($theme_key, $region = NULL) {
  static $all_blocks = NULL;
  static $records = NULL;

  $blocks = array();

  // Load the blocks and regions if we haven't yet.
  if (is_null($all_blocks)) {
    $all_blocks = _region_manager_block_rehash($theme_key);
  }
  if (is_null($records)) {
    // Load all regions for this theme, so we don't have to reload for each region.
    $records = region_manager_regions_load($theme_key);
  }

  // Might as well quit if we don't have any blocks.
  if (empty($all_blocks)) {
    return;
  }

  if (is_null($region)) {
    foreach(array_keys(region_manager_region_list($theme_key)) as $region) {
      $blocks[$region] = _region_manager_blocks_filter($region, $all_blocks, $records[$region]);
    }
  }
  else {
    $blocks = _region_manager_blocks_filter($region, $all_blocks, $records[$region]);
  }

  return $blocks;
}

/**
 * Returns an array of links to create new blocks in this region.
 *
 * @param $theme_key
 *   A string containing the key of the active theme.
 * @param $region
 *   A string containing the region to be managed.
 * @return
 *   An array containing the links.
 */
function region_manager_create_links($theme_key, $region) {
  $links = $options = array();
    
  if ($path = $_REQUEST['destination']) {
    $options['query'] = array(
      'destination' => $path,
      'theme_key' => $theme_key,
      'region' => $region,
    );
  }

  foreach(region_manager_create_links_load($theme_key, $region) as $link) {
    if ($link['options']) {
      $options += $link['options'];
    }
    $links[] = l($link['title'], $link['path'], $options);
  }
  return $links;
}

/**
 * Retrieves an array of link items, invoking hook_region_manager_create_links()
 * and hook_region_manager_create_links_alter().  The links are cached and
 * static to ensure optimum performance.
 *
 * @param $theme_key
 *   A string containing the key of the active theme.
 * @param $region
 *   A string containing the region to be managed.
 * @param $reset
 *   Boolean, if TRUE the cache will be flushed.
 * @return
 *   An array containing the links.
 */
function region_manager_create_links_load($theme_key, $region, $reset = FALSE) {
  static $links;

  if (is_null($links) || $reset) {
    if ($reset) {
      cache_clear_all('region_manager_create_links', 'cache');
    }

    if ($cache = cache_get('region_manager_create_links')) {
      $links = $cache->data;
    }
    else {
      $links = module_invoke_all('region_manager_create_links', $theme_key, $region);
      drupal_alter('region_manager_create_links', $links, $theme_key, $region);
      cache_set('region_manager_create_links', $links, 'cache', CACHE_TEMPORARY);
    }
  }

  return $links;
}

/**
 * Helper function to filter all enabled and (available) disabled
 * blocks for a particular region.
 */
function _region_manager_blocks_filter($region, $blocks = array(), $record = array()) {
  $output = array();

  foreach($blocks as $module => $modblocks) {
    if (is_array($modblocks)) {

      // If they're all disabled for this module, go on to the next module.
      if ($record['modules'][$module] == REGION_MANAGER_ALL_DISABLED) {
        continue;
      }

      // Otherwise, let's parse through each block.
      else {
        foreach($modblocks as $delta => $block) {
          if ($block['status']) {
            // Only add it to active if it's a part of this region.
            if ($block['region'] == $region) {
              $output[] = $block;
            }
          }
          else {
            // Check to see if we're using custom availability for this module. 
            if ($record['modules'][$module] == REGION_MANAGER_CUSTOM) {
              $key = $module .':'. $delta;
              // Is this block 'blocked'?
              if (!in_array($key, $record['blocked_blocks'])) {
                $output[] = $block;
              }
            }
            // Otherwise add all blocks for this module.
            else {
              $output[] = $block;
            }
          }
        }
      }
    }
  }

  return $output;
}

/**
 * Helper function to assemble the blocks.
 */
function _region_manager_block_rehash($theme_key) {
  $blocks = array();
 
  $result = db_query("SELECT * FROM {blocks} WHERE theme = '%s'", $theme_key);
  $old_blocks = array();
  while ($old_block = db_fetch_array($result)) {
    $old_blocks[$old_block['module']][$old_block['delta']] = $old_block;
  }

  $blocks = array();
  // Valid region names for the theme.
  $regions = system_region_list($theme_key);

  foreach (module_list() as $module) {
    $module_blocks = module_invoke($module, 'block', 'list');
    if ($module_blocks) {
      foreach ($module_blocks as $delta => $block) {
        if (empty($old_blocks[$module][$delta])) {
          // If it's a new block, add identifiers.
          $block['module'] = $module;
          $block['delta']  = $delta;
          $block['theme']  = $theme_key;
          if (!isset($block['pages'])) {
            // {block}.pages is type 'text', so it cannot have a
            // default value, and not null, so we need to provide
            // value if the module did not.
            $block['pages']  = '';
          }
          // Add defaults and save it into the database.
          drupal_write_record('blocks', $block);
          // Set region to none if not enabled.
          $block['region'] = $block['status'] ? $block['region'] : BLOCK_REGION_NONE;
          // Add to the list of blocks we return.
          $blocks[$module][$delta] = $block;
        }
        else {
          // If it's an existing block, database settings should overwrite
          // the code. But aside from 'info' everything that's definable in
          // code is stored in the database and we do not store 'info', so we
          // do not need to update the database here.
          // Add 'info' to this block.
          $old_blocks[$module][$delta]['info'] = $block['info'];
          // If the region name does not exist, disable the block and assign it to none.
          if (!empty($old_blocks[$module][$delta]['region']) && !isset($regions[$old_blocks[$module][$delta]['region']])) {
            drupal_set_message(t('The @block %info was assigned to the invalid region %region and has been disabled.', array('@block' => _region_manager_block_name(), '%info' => $old_blocks[$module][$delta]['info'], '%region' => $old_blocks[$module][$delta]['region'])), 'warning');
            $old_blocks[$module][$delta]['status'] = 0;
            $old_blocks[$module][$delta]['region'] = BLOCK_REGION_NONE;
          }
          else {
            $old_blocks[$module][$delta]['region'] = $old_blocks[$module][$delta]['status'] ? $old_blocks[$module][$delta]['region'] : BLOCK_REGION_NONE;
          }
          // Add this block to the list of blocks we return.
          $blocks[$module][$delta] = $old_blocks[$module][$delta];
          // Remove this block from the list of blocks to be deleted.
          unset($old_blocks[$module][$delta]);
        }
      }
    }
  }

  // Remove blocks that are no longer defined by the code from the database.
  foreach ($old_blocks as $module => $old_module_blocks) {
    foreach ($old_module_blocks as $delta => $block) {
      db_query("DELETE FROM {blocks} WHERE module = '%s' AND delta = '%s' AND theme = '%s'", $module, $delta, $theme_key);
    }
  }
  return $blocks;
}

/**
 * Helper function to determine if the block is visible for a certain path.
 *
 * @return
 *   Boolean FALSE if it's not visible, otherwise a string of the path.
 */
function _region_manager_block_check_visibility($block, $path = NULL) {
  if ($block['visibility'] == 1 || $block['visibility'] == -1) {
    $path = is_null($path) ? drupal_get_path_alias($_GET['q']) : $path;
    // Compare with the internal and path alias (if any).
    $page_match = drupal_match_path($path, $block['pages']);
    if ($path != $_GET['q']) {
      $page_match = $page_match || drupal_match_path($_GET['q'], $block['pages']);
    }
  }
  return $page_match ? $path : FALSE;
}

/**
 * Helper function to display the custom block name.
 *
 * @param $name
 *   String designating upper or lower case.
 */
function _region_manager_block_name($case = 'lower') {
  static $name;

  if (empty($name)) {
    $name = variable_get('region_manager_block_name', 'Block');
  }

  if ($case == 'lower') {
    return strtolower($name);
  }
  return ucfirst($name);
}

/**
 * Helper function to output a path name or alias.
 */
function _region_manager_path_name($path) {
  $frontpage = variable_get('site_frontpage', 'node');
  $alias = ($path == $frontpage) ? t('home page') : drupal_get_path_alias($path);

  return $alias;
}
