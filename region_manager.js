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
};

/**
 * Perform actions from operation links
 */
Drupal.behaviors.regionManagerOperations = function() {
  $('td.operations a').click(function() {
    var op = $(this).attr('class');
    var rowClass = $(this).parents('ul').attr('class');
    var row = $('tr.' + rowClass);
    var configWrapper = $('div.region-manager-block-title-wrapper', row);
    var tableDragObj = Drupal.tableDrag['region-manager-blocks-active_path'];

    switch(op) {
      case 'add':
        row.addClass('draggable').insertAfter('table#region-manager-blocks-active_path tr.rm-state-message');
        tableDragObj.makeDraggable(row);
        $('select.block-state-select', row).val('active_path');
        break;
        
      case 'configure':
        if (configWrapper.is(':hidden')) {
          configWrapper.show('fast');
        }
        else {
          configWrapper.hide('fast');
        }
        break;

      case 'remove':
        row.removeClass('draggable').insertAfter('table#region-manager-blocks-active tr.rm-state-message');
        $('select.block-state-select', row).val('active');
        $('a.tabledrag-handle', row).remove();
        configWrapper.hide();
        break;

      case 'disable':
        row.removeClass('draggable').insertAfter('table#region-manager-blocks-disabled tr.rm-state-message');
        $('select.block-state-select', row).val('disabled');
        $('a.tabledrag-handle', row).remove();
        configWrapper.hide();
        break;
    }
      $('tr', $('#region-manager-manage-form table'))
    .filter(':odd').filter('.odd')
      .removeClass('odd').addClass('even')
    .end().end()
    .filter(':even').filter('.even')
      .removeClass('even').addClass('odd');

    return false;
  });

  
}