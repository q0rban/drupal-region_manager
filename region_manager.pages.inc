<?php
// $Id$

/**
 * @file
 *   Page callbacks for managing regions.
 */

/**
 * 
 */
function region_manager_manage_form(&$form_state, $theme_key, $region = NULL) {
  $form = array();

  $blocks = _region_manager_block_rehash($theme_key, $region);

  return $form;
}