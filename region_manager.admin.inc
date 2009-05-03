<?php
// $Id$

/**
 * @file
 *   Holds all administration forms for region_manager.
 */

/**
 * Region settings form.
 */
function region_manager_regions_form() {
  $form = array();

  drupal_clear_css_cache();
  $themes = system_theme_data();
  $settings = variable_get('region_manager_regions', array());

  module_load_include('inc', 'system', 'system.admin');
  uasort($themes, 'system_sort_modules_by_info_name');

  $form['region_manager_regions']['#tree'] = TRUE;

  foreach($themes as $theme) {
    if ($theme->status) {
      $form['region_manager_regions'][$theme->name] = array(
        '#type' => 'fieldset',
        '#title' => $theme->info['name'],
        '#tree' => TRUE,
      );
  
      foreach (system_region_list($theme->name) as $region => $title) {
        $form['region_manager_regions'][$theme->name][$region] = array(
          '#type' => 'checkbox',
          '#title' => $title,
          '#default_value' => $settings[$theme->name][$region],
        );
      }
    }
  }

  $form = system_settings_form($form);

  // Uncomment and update the callback to override the default system settings submit handler.
  // $form['submit'] = array('region_manager_regions_form_submit');

  return $form;
}

/**
 * Block settings form.
 */
function region_manager_blocks_form(&$form_state, $theme_key) {
  $form = array();
 
  $roles = user_roles();
  $regions = region_manager_region_list($theme_key);
  $disabled = region_manager_regions_load($theme_key);

  if (empty($regions)) {
    drupal_set_message(t('You must first enable some regions for this theme <a href="!href">here</a>.', array('!href' => 'admin/settings/region_manager')), 'error');
    return array();
  }

  $form['#theme_key'] = $theme_key;
  $form['blocks']['#tree'] = TRUE;

  $blocks_form = array('#tree' => TRUE);

  foreach(array_keys($regions) as $region) {
    $form[$region]['#tree'] = TRUE;
    foreach(_region_manager_block_rehash($theme_key) as $module => $blocks) {
  
      $form['blocks']['block_names'][$module] = array(
        '#type' => 'markup',
        '#value' => t('all @module blocks', array('@module' => $module)),
      );
  
      $form['blocks'][$region][$module] = array(
        '#type' => 'select',
        '#default_value' => 1, // @todo: have this load the val from the table.
        '#options' => array(
          REGION_MANAGER_ALL_ENABLED => t('All enabled'),
          REGION_MANAGER_ALL_DISABLED => t('All disabled'),
          REGION_MANAGER_CUSTOM => t('Custom per block'),
        ),
      );

      $disabled = ($form['blocks'][$region][$module]['#default_value'] != REGION_MANAGER_CUSTOM);
      foreach($blocks as $delta => $block) { 
        // Store the block name in a separate form.
        $key = $module .':'. $delta;
        $form['blocks']['block_names'][$key] = array(
          '#type' => 'markup',
          '#value' => $block['info'],
        );
  
        $form['blocks'][$region][$key] = array(
          '#type' => 'checkbox',
          '#default_value' => !in_array($key, (array) $disabled[$region]),
          '#disabled' => $disabled,
        );
      }
    }
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));
 
  return $form;
}
 
 
/**
 * Submit handler for region_manager_blocks_form
 */
function region_manager_blocks_form_submit($form, &$form_state) {
  $regions = $form_state['values']['blocks'];
  $theme_key = $form['#theme_key'];

  ob_start();
  print_r($blocks);
  drupal_set_message(ob_get_contents());
  ob_end_clean();

  foreach($regions as $region => $blocks) {
    foreach($blocks as $key => $value) {
      list($module, $delta) = explode(':', $key);
      if (!isset($delta)) {
      
      }
      else {
      
      }
    }
  }

  drupal_set_message(t('The changes have been saved.'));
}
 
/**
 * Theme the region_manager_blocks_form.
 *
 * @ingroup themeable
 */
function theme_region_manager_blocks_form($form) {
  if (empty($form['blocks'])) {
    return;
  }
  $output = '';

  $regions = region_manager_region_list($form['#theme_key']);

  $header = array();
  $header[] = (t('Block'));
  foreach($regions as $region => $title) {
    $header[] = $title;
  }
 
  foreach(element_children($form['blocks']) as $region) {
    foreach(element_children($form['blocks'][$region]) as $key) {
      list($module, $delta) = explode(':', $key);
      $data = drupal_render($form['blocks'][$region][$key]);
      $classes = 'region_manager_module-'. $module .' region_manager_region-'. $region;
      $rows[$key]['data'][$region] = array('data' => $data, 'class' => $classes);
      if (!isset($rows[$key]['class'])) {
        if (isset($delta)) {
          $rows[$key]['class'] = $module .'-child block';
        }
        else {
          $rows[$key]['class'] = $module .' module';
        }
      }
    }
  }

  $output .= theme('table', $header, $rows, array('id' => 'regionmanager-regions'));
  $output .= drupal_render($form);
 
  return $output;
}

/**
 * Helper function to assemble the blocks 
 */
function _region_manager_block_rehash($theme_key) {
  // Valid region names for the theme.
  $regions = region_manager_region_list($theme_key);

  foreach (module_list() as $module) {
    $module_blocks = module_invoke($module, 'block', 'list');
    if ($module_blocks) {
      foreach ($module_blocks as $delta => $block) {
        $block['module'] = $module;
        $block['delta']  = $delta;
        $block['theme']  = $theme_key;
        if (!isset($block['pages'])) {
          // {block}.pages is type 'text', so it cannot have a
          // default value, and not null, so we need to provide
          // value if the module did not.
          $block['pages']  = '';
        }
        // Set region to none if not enabled.
        $block['region'] = $block['status'] ? $block['region'] : BLOCK_REGION_NONE;
        // Add to the list of blocks we return.
        $blocks[$module][$delta] = $block;
      }
    }
  }

  return $blocks;
}