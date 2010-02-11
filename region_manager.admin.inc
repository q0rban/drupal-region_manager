<?php
// $Id: region_manager.admin.inc,v 1.2 2010/02/09 00:13:20 q0rban Exp $

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

  return system_settings_form($form);
}

/**
 * Region settings form.
 */
function region_manager_regions_form() {
  $form = array();

  drupal_clear_css_cache();
  $themes = system_theme_data();

  module_load_include('inc', 'system', 'system.admin');
  uasort($themes, 'system_sort_modules_by_info_name');

  $form['region_manager_regions']['#tree'] = TRUE;

  foreach($themes as $theme) {
    $key = 'region_manager_regions_'. $theme->name;
    $settings = region_manager_regions_variable_get($theme->name);

    if ($theme->status) {
      $form[$key] = array(
        '#type' => 'fieldset',
        '#title' => $theme->info['name'],
        '#tree' => TRUE,
      );
  
      foreach (system_region_list($theme->name) as $region => $title) {
        $form[$key][$region] = array(
          '#type' => 'checkbox',
          '#title' => $title,
          '#default_value' => $settings[$region],
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
  $records = region_manager_regions_load($theme_key, TRUE);
  $all_blocks = _region_manager_block_rehash($theme_key);

  if (empty($regions)) {
    drupal_set_message(t('You must first enable some regions for this theme <a href="!href">here</a>.', array('!href' => 'admin/settings/region_manager')), 'error');
    return array();
  }

  $form['#theme_key'] = $theme_key;
  $form['blocked_blocks']['#tree'] = TRUE;

  $form['regions']['#tree'] = TRUE;

  $storage = REGION_MANAGER_STORAGE_NORMAL;

  foreach(array_keys($regions) as $region) {
    // Check the storage on this region.
    if ($records[$region]['storage'] > $storage) {
      $storage = $records[$region]['storage'];
    }

    // Now build each module form.
    foreach($all_blocks as $module => $blocks) {
      // Add the label for this module to the block_names column.
      $form['regions']['block_names']['modules'][$module] = array(
        '#type' => 'markup',
        '#value' => t('all @module blocks', array('@module' => $module)),
      );

      // The select box for the module.
      $form['regions'][$region]['modules'][$module] = array(
        '#type' => 'select',
        '#default_value' => isset($records[$region]['modules'][$module]) ? $records[$region]['modules'][$module] : REGION_MANAGER_ALL_ENABLED,
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
          '#default_value' => !in_array($key, (array) $records[$region]['blocked_blocks']),
        );

        // Set tree to TRUE.
        $form['regions'][$region]['blocked_blocks'][$module]['#tree'] = $form['regions'][$region]['blocked_blocks'][$module][$delta]['#tree'] = TRUE;
      }
    }
    // Set tree to true for these arrays.
    $form['regions'][$region]['#tree'] = $form['regions'][$region]['modules']['#tree'] = TRUE;

    // Now set the rid for this region.
    if (isset($records[$region]['rid'])) {
      $form['regions'][$region]['rid'] = array(
        '#type' => 'hidden',
        '#value' => $records[$region]['rid'],
      );
    }
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

  switch($storage) {
    case REGION_MANAGER_STORAGE_OVERRIDE:
      $form['reset'] = array('#type' => 'submit', '#value' => t('Reset'));
      break;

    case REGION_MANAGER_STORAGE_DEFAULT:
      drupal_set_message(t('These regions are currently using default storage. Any changes submitted here will override the default values.'), 'warning');
      $form['submit']['#value'] = t('Override');
      break;
  }

  return $form;
}

/**
 * Submit handler for region_manager_blocks_form
 */
function region_manager_blocks_form_submit($form, &$form_state) {
  $regions = $form_state['values']['regions'];
  $theme_key = $form['#theme_key'];

  // If we're resetting, delete all from the db with this theme_key.
  if ($form_state['values']['op'] == t('Reset')) {
    $sql = "DELETE FROM {region_manager_regions} WHERE theme_key = '%s'";
    db_query($sql, $theme_key);
  }
  else {
    foreach($regions as $region => $record) {
      // Set up the region and theme key for this record.
      $record['region'] = $region;
      $record['theme_key'] = $theme_key;
  
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
  }

  cache_clear_all('region_manager:'. $theme_key, 'cache', TRUE);
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

/**
 * List of themes for 
 */
function region_manager_visibility_overview() {
  $items = array();

  foreach(list_themes() as $theme) {
    if ($theme->status) {
      $items[] = l(t($theme->info['name']), 'admin/settings/region_manager/visibility/'. $theme->name);
    }
  }

  return theme('item_list', $items, t('Themes'), 'ul', array('class' => 'region-manager-visibility-overview'));
}

/**
 * Visibility settings form.
 */
function region_manager_visibility_form(&$form_state, $theme_key) {
  $form = array();
  $first = TRUE;

  foreach(region_manager_region_list($theme_key) as $region => $name) {
    $key = $theme_key .':'. $region;
    $record = $records[$region];

    visibility_api_form($form, $key, 'region_manager');

    $form['visibility_api'][$key]['#type'] = 'fieldset';
    $form['visibility_api'][$key]['#title'] = $name;
    $form['visibility_api'][$key]['#collapsible'] = TRUE;
    $form['visibility_api'][$key]['#collapsed'] = !$first;

    $first = FALSE;
  }

  visibility_api_form_prepare($form);

  return $form;
}

/**
 * Form to export region manager settings.
 */
function region_manager_export(&$form_state) {
  $form = array();
  $values = $_POST;

  if ($values) {
    return region_manager_export_results($values);
  }

  $form['themes']['#tree'] = TRUE;

  foreach(system_theme_data() as $theme) {
    $regions = array_filter(region_manager_regions_variable_get($theme->name));
    if (!empty($regions)) {
      $options[$theme->name] = $theme->info['name'];
    }
  }

  $form['themes'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Themes'),
    '#options' => $options,
  );

  $form['module_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Module name'),
    '#description' => t('The short name of your module, used to prepend the hook.'),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

  return $form;
}

function region_manager_export_results($values) {
  $form = $options = $export = array();
  $themes = array_keys(array_filter($values['themes']));
  $module_name = $values['module_name'] ? check_plain($values['module_name']) : 'hook';
  $records = region_manager_regions_load_all();

  foreach($records as $record) {
    if (in_array($record['theme_key'], $themes)) {
      unset($record['storage'], $record['rid']);
      $export[$record['theme_key']][$record['region']] = $record;
    }
  }

  $output  = "/**\n";
  $output .= " * Implementation of hook_region_manager_defaults().\n";
  $output .= " */\n";
  $output .= "function {$module_name}_region_manager_defaults(" . '$theme_key' . " = NULL) {\n";
  $output .= '  $records = array();' . "\n";
  $output .= "\n";
  $output .= '  $records = '. var_export($export, TRUE) .";\n";
  $output .= "\n";
  $output .= '  return $theme_key ? $records[$theme_key] : $records;';
  $output .= "\n}";

  $rows = substr_count($output, "\n") + 1;

  $form['export'] = array(
    '#type' => 'textarea',
    '#resizable' => FALSE,
    '#rows' => $rows,
    '#default_value' => $output,
  );

  return $form;
}