// $Id: region_manager.js,v 1.1.2.3 2009/05/13 18:56:06 q0rban Exp $

/**
 * Show/hide form elements for editing a block title.
 */
Drupal.behaviors.blockTitle = function() {
  // Show hide the textfield depending on the radio states.
  $('input.region-manager-manage-form-title-status').change(function() {
    if (this.checked) {
      var textfield = $('input.form-text', $(this).parents('td.block'));
      switch (this.value) {
        // Default Title
        case '0':
          textfield.val('').hide('fast');
          break;

        // Disable Title
        case '1':
          textfield.val('<none>').hide();
          break;

        // Custom Title
        case '2':
          if (textfield.val() == '<none>') {
            textfield.val('');
          }
          textfield.show('fast');
          break;
      }
    }
  });

  // Instantiate a change so we can hide the title field if need be.
  $('input.region-manager-manage-form-title-status').change();

  $('td.operations a').click(function() {
    var className = $(this).attr('class');
    var wrapper = $('div.' + className);
    if (wrapper.is(':hidden')) {
      wrapper.show('fast');
    }
    else {
      wrapper.hide('fast');
    }
    return false;
  });
};