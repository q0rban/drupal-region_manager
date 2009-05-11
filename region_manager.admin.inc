<?php
// $Id$

/**
 * @file
 *   Holds all administration forms for region_manager.
 */

/**
 * Configuration settings form.
 */
function region_manager_settings_form() {
  $form = array();

  $form['region_manager_block_name'] = array(
    '#type' => 'textfield',
    '#title' => t('What should a "Block" be called?'),
    '#description' => t('Please note: this only changes the name "Block" within the region manager UI, not throughout your Drupal site.'),
    '#default_value' => variable_get('region_manager_block_name', 'Block'),
  );

  $form = system_settings_form($form);

  // Uncomment and update the callback to override the default system settings submit handler.
  // $form['#submit'] = array('_settings_form_submit');

  return $form;
}

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
  $records = region_manager_regions_load($theme_key);

  if (empty($regions)) {
    drupal_set_message(t('You must first enable some regions for this theme <a href="!href">here</a>.', array('!href' => 'admin/settings/region_manager')), 'error');
    return array();
  }

  $form['#theme_key'] = $theme_key;
  $form['blocked_blocks']['#tree'] = TRUE;

  $form['regions']['#tree'] = TRUE;

  foreach(array_keys($regions) as $region) {
    // rid is the theme key and region delimited by a colon.
    $rid = $theme_key .':'. $region;

    // Now build each module form.
    foreach(_region_manager_block_rehash($theme_key) as $module => $blocks) {

      // Add the label for this module to the block_names column.
      $form['regions']['block_names']['modules'][$module] = array(
        '#type' => 'markup',
        '#value' => t('all @module blocks', array('@module' => $module)),
      );

      // The select box for the module.
      $form['regions'][$region]['modules'][$module] = array(
        '#type' => 'select',
        '#default_value' => isset($records[$rid]['modules'][$module]) ? $records[$rid]['modules'][$module] : REGION_MANAGER_ALL_ENABLED,
        '#options' => array(
          REGION_MANAGER_ALL_ENABLED => t('All enabled'),
          REGION_MANAGER_ALL_DISABLED => t('All disabled'),
          REGION_MANAGER_CUSTOM => t('Custom per block'),
        ),
      );

      // Now, build the checkboxes for each blocks.
      foreach($blocks as $delta => $block) {
        // Add the label for this block to the block_names column.
        $key = $module .':'. $delta;
        $form['regions']['block_names']['blocked_blocks'][$module][$delta] = array(
          '#type' => 'markup',
          '#value' => $block['info'],
        );

        // The checkbox for the block.
        $form['regions'][$region]['blocked_blocks'][$module][$delta] = array(
          '#type' => 'checkbox',
          '#default_value' => !in_array($key, (array) $records[$rid]['blocked_blocks']),
        );

        // Set tree to TRUE.
        $form['regions'][$region]['blocked_blocks'][$module]['#tree'] = $form['regions'][$region]['blocked_blocks'][$module][$delta]['#tree'] = TRUE;
      }
    }
    // Set tree to true for these arrays.
    $form['regions'][$region]['#tree'] = $form['regions'][$region]['modules']['#tree'] = TRUE;
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

  return $form;
}
 
 
/**
 * Submit handler for region_manager_blocks_form
 */
function region_manager_blocks_form_submit($form, &$form_state) {
  $regions = $form_state['values']['regions'];
  $theme_key = $form['#theme_key'];

  foreach($regions as $region => $record) {
    // Set the rid for this record.
    $record['rid'] = $theme_key .':'. $region;

    // Now go through the modules.
    foreach($record['modules'] as $module => $module_val) {
      foreach($record['blocked_blocks'][$module] as $delta => $block_val) {
        // We were using the [module][delta] structure to ease in theming, but we
        // need it to be set with just a module:delta key, so let's do that now.
        $key = $module .':'. $delta;
        $record['blocked_blocks'][$key] = $block_val;

        // Now we switch through the $module_val to determine what needs to be done.
        switch($module_val) {
          case REGION_MANAGER_ALL_ENABLED:
            // All are enabled, so unset all.
            unset($record['blocked_blocks'][$key]);
            break;

          case REGION_MANAGER_CUSTOM:
            // We don't want to save the checked blocks, only unchecked blocks.
            if ($block_val) {
              unset($record['blocked_blocks'][$key]);
            }
            break;

          case REGION_MANAGER_ALL_DISABLED:
            // Nothing to do here, since we want all of them.
            break;
        }
      }
      // Unset this module's array since we resaved using the module:delta format.
      unset($record['blocked_blocks'][$module]);
    }
    // Flip the array, as all we need are the keys at this point.
    $record['blocked_blocks'] = implode(', ', array_keys($record['blocked_blocks']));
    region_manager_region_save($record);
  }

  drupal_set_message(t('The changes have been saved.'));
}
 
/**
 * Theme the region_manager_blocks_form.
 *
 * @ingroup themeable
 */
function theme_region_manager_blocks_form($form) {
  if (empty($form['blocked_blocks'])) {
    return;
  }

  drupal_add_js(drupal_get_path('module', 'region_manager') .'/region_manager.admin.js');
  $output = '';

  $regions = region_manager_region_list($form['#theme_key']);

  $header = array();
  $header[] = (t('Block'));
  foreach($regions as $region => $title) {
    $header[] = $title;
  }
 
  foreach(element_children($form['regions']) as $region) {
    foreach(element_children($form['regions'][$region]['modules']) as $module) {
      $data = drupal_render($form['regions'][$region]['modules'][$module]);
      $classes = 'region_manager_module-'. $module .' region_manager_region-'. $region;
      $rows[$module]['data'][$region] = array('data' => $data, 'class' => $classes);
      if (!isset($rows[$module]['class'])) {
        $rows[$module]['class'] = $module .' module';
      }
      foreach(element_children($form['regions'][$region]['blocked_blocks'][$module]) as $delta) {
        $key = $module .':'. $delta;
        $data = drupal_render($form['regions'][$region]['blocked_blocks'][$module][$delta]);
        $classes = 'region_manager_module-'. $module .' region_manager_region-'. $region;
        $rows[$key]['data'][$region] = array('data' => $data, 'class' => $classes);
        if (!isset($rows[$key]['class'])) {
          $rows[$key]['class'] = $module .'-child block';
        }
      }
    }
  }

  $output .= theme('table', $header, $rows, array('id' => 'regionmanager-regions'));
  $output .= drupal_render($form);
 
  return $output;
}