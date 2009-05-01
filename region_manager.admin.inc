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
  $disabled = region_manager_blocks_theme_load($theme_key);

  $form['#theme_key'] = $theme_key;
  $form['blocks']['#tree'] = TRUE;

  foreach(_region_manager_block_rehash($theme_key) as $module => $blocks) {
    $all_blocks = array('all_blocks' => array('info' => t('all @module blocks', array('@module' => $module))));
    $blocks = $all_blocks + $blocks;

    foreach($blocks as $delta => $block) {

      // Store the block name in a separate form.
      $key = $module .'_'. $delta;
      $form['block_names'][$key] = array(
        '#type' => 'markup',
        '#value' => $block['info'],
      );

      $form['blocks'][$module][$delta]['#tree'] = TRUE;
      foreach(array_keys($regions) as $region) {
        $form['blocks'][$module][$delta][$region] = array(
          '#type' => 'checkbox',
          '#default_value' => !in_array($key, (array) $disabled[$region]),
          '#disabled' => (isset($form['blocks'][$module]['all_blocks'][$region]) && $form['blocks'][$module]['all_blocks'][$region]['#default_value']),
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
  $blocks = $form_state['values']['blocks'];
  $theme_key = $form['#theme_key'];

  ob_start();
  print_r($blocks);
  drupal_set_message(ob_get_contents());
  ob_end_clean();

  foreach($blocks as $module => $blocks) {
    foreach($blocks as $delta => $regions) {
      $all = array;
      foreach($regions as $region => $value) {
        if ($delta == 'all_blocks') {
          if ($value) {
            // Delete all blocks for this module and region.
          }
          else {
            // Add all blocks for this module and region.
          }
        }
        elseif ($value) {
          // Delete block $delta for this region.
        }
        else {
          // Disable this block for this region by adding it to our disabled blocks table.
        }
      }
    }
  }


  //foreach ($blocks as $key => $regions) {
    //foreach($regions)
  //}

  drupal_set_message(t('The changes have been saved.'));
}
 
/**
 * Theme the region_manager_blocks_form.
 *
 * @ingroup themeable
 */
function theme_region_manager_blocks_form($form) {
  $output = '';

  $regions = region_manager_region_list($form['#theme_key']);

  $header = array();
  $header[] = (t('Block'));
  foreach($regions as $region => $title) {
    $header[] = $title;
  }
 
  foreach(element_children($form['blocks']) as $module) {
    $rows = array();
    foreach(element_children($form['blocks'][$module]) as $delta) {
      $row = array();
      $row[] = drupal_render($form['block_names'][$module .'_'. $delta]);
      foreach(element_children($form['blocks'][$module][$delta]) as $region) {
        $data = drupal_render($form['blocks'][$module][$delta][$region]);
        $classes = 'region_manager_module-'. $module .' region_manager_region-'. $region;
        $row[] = array('data' => $data, 'class' => $classes);
      }
      if ($delta == 'all_blocks') {
        $rows[] = array('data' => $row, 'class' => 'module');
      }
      else {
        $rows[] = $row;
      }
    }
    $output .= theme('table', $header, $rows, array('id' => 'regionmanager-regions-'. $module));
  }

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