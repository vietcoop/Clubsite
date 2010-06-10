<?php
// $Id$

/**
 * @file
 * User-code admin forms.
 */

define('W_USER_CODE_INVITATION_SUBJECT_TEMPLATE', '');
define('W_USER_CODE_INVITATION_BODY_TEMPLATE', '');

/**
 * User code settings form.
 */
function w_user_code_settings_form(&$form_state) {
  $form['fields']['#tree'] = TRUE;
  
  $settings = variable_get('wUserCodeFieldsVisibility', array());
  
  $q = db_query(
    'SELECT title, name, type, category, fid, weight'
      . ' FROM {profile_fields}'
      . ' WHERE register = 1'
      . ' ORDER BY category, weight'
  );
  
  while ($field = db_fetch_object($q)) {
    $form['fields'][$field->name] = array(
      '#type'    => 'radios',
      '#field'   => $field,
      '#options' => array(
        'none'     => '',
        'use-code' => '',
        'not-use'  => '',
        'both'     => '',
      ),
      '#default_value' => isset($settings[$field->name]) ? $settings[$field->name] : 'both',
    );
  }

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Theming function for user-code settings form.
 */
function theme_w_user_code_settings_form(&$form) {
  $output = '';

  $rows = array();
  foreach (element_children($form['fields']) as $fid) {
    $e = &$form['fields'][$fid];
    $field = &$e['#field'];
    $rows[] = array(
      $field->category,
      $field->type,
      $field->name,
      drupal_render($e['none']),
      drupal_render($e['use-code']),
      drupal_render($e['not-use']),
      drupal_render($e['both']),
    );
  }
  
  $output .= '<h3>Field visibility on user register form</h3>'; 
  $output .= theme(
    'table', 
    array(t('Category'), t('Type'), t('Name'), t('None'), t('Use code'), t('Not use'), t('Both')),
    $rows
  );
  
  return $output . drupal_render($form);
}

/**
 * Submit handler for userCode settings form.
 */
function w_user_code_settings_form_submit(&$form, &$form_state) {
  variable_set('wUserCodeFieldsVisibility', $form_state['values']['fields']);
}

/**
 * Form to send invitation.
 */
function w_user_code_invite_form(&$form_state) {
  $form['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('Email address'),
    '#required' => TRUE,
    '#description' => t('Email address of who you would like to send the invitation to.'),
  );
  
  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject template'),
    '#required' => TRUE,
    '#default_value' => variable_get('wUserCodeInvitationSubjectTemplate', W_USER_CODE_INVITATION_SUBJECT_TEMPLATE),
    '#description' => t('Email subject template.'),
  );

  $form['body'] = array(
    '#type' => 'textarea',
    '#title' => t('Body template'),
    '#default_value' => variable_get('wUserCodeInvitationBodyTemplate', W_USER_CODE_INVITATION_BODY_TEMPLATE),
    '#required' => TRUE,
    '#rows' => 7,
    '#description' => t('Email body template is used for outgoing email.'),
  );

  $form['available_tokens'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available tokens'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#value' => theme('w_global_tokens_list')
  );

  $form['save_template'] = array(
    '#type' => 'checkbox',
    '#title' => t('Save email template'),
    '#default_value' => TRUE,
  );
  
  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function w_user_code_invite_form_validate(&$form, &$form_state) {
  $v = &$form_state['values'];
  
  // Validate email address
  if (!valid_email_address($v['mail'])) {
    form_set_error('mail', t('Invalid email address.'));
  }
  else {
    // Is there a existing user used this email address
    $counter = db_result(db_query(
      "SELECT COUNT(*) FROM {users} WHERE mail = '%s'",
      $v['mail']
    ));
    
    if ($counter) {
      form_set_error(
        'mail',
        t('Email address is already used by an existing user.')
      );
    }
  }
}

function w_user_code_invite_form_submit(&$form, &$form_state) {
  global $user;
  $v = &$form_state['values'];

  if ($v['save_template']) {
    variable_set('wUserCodeInvitationSubjectTemplate', $v['subject']);
    variable_set('wUserCodeInvitationBodyTemplate', $v['body']);
  }

  $params = array();
  $params['subject'] = $v['subject'];
  $params['body']    = $v['body'];

  drupal_mail(
    'w', 
    'userCodeInvitation', 
    $v['mail'],
    user_preferred_language($user),
    $params
  );
}

/** 
 * Helper function to send invitation-mail.
 */
function _w_mail_user_code_invitation_mail(&$message, $params) {
  $message['subject'] = check_plain($params['subject']);
  $message['body'][]  = drupal_html_to_text(token_replace($params['body']));
}
