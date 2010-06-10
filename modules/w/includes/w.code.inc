<?php
// $Id$

/**
 * @file Functions to manage userCode.
 */

define('W_COOKIE_USER_CODE', 'wUserCode');

 /**
  * Check is the userCode already used.
  */
function w_user_code_is_duplicated($code) {
  $counter = db_result(db_query(
    "SELECT COUNT(*) FROM {w_user_code} WHERE code LIKE '%s'",
    $code
  ));

  return $counter > 0;
}

function w_user_code_is_valid_format($code) {
  $sub = "[0-9A-F]{4}";
  return preg_match("/^{$sub}-{$sub}-{$sub}-{$sub}$/", $code);
}

function w_user_code_in_queue($code) {
  return (bool)db_result(db_query(
    "SELECT COUNT(*) FROM {w_user_code} WHERE code LIKE '%s' AND uid = 0",
    $code
  ));
}

function w_user_code_used($code) {
  return (bool)db_result(db_query(
    "SELECT COUNT(*) FROM {w_user_code} WHERE code LIKE '%s' AND uid <> 0",
    $code
  ));
}

function w_user_code_validate($code) {
  if (!w_user_code_is_valid_format($code)) {
    return t('Invalid code format.');
  }

  if (w_user_code_used($code)) {
    return t('Code is already used.');
  }

  if (!w_user_code_in_queue($code)) {
    return t('Code not found.');
  }

  return TRUE;
}

/**
 * Get userCode by userID.
 */
function w_user_code($uid) {
  return db_result(db_query("SELECT code FROM {w_user_code} WHERE uid = %d", $uid));
}

function w_user_code_remove($uid) {
  db_query("DELETE FROM {w_user_code} WHERE uid = %d", $uid);
}

/**
 * Generate the random code look like: XXXX-XXXX-XXXX-XXXX.
 */
function w_generate_user_code($length = 16, $no_duplicate = TRUE, $insert = FALSE) {
  $chars = "0123456789ABCDEF";
  $size  = strlen($chars);
  $code  = '';
  $counter = 1;
  while ($counter <= $length) {
    $code .= substr($chars, rand(0, $size - 1), 1);
    if ($counter < $length && $counter % 4 === 0) {
      $code .= '-';
    }

    $counter++;
  }

  if ($no_duplicate) {
    if (w_user_code_is_duplicated($code)) {
      return w_generate_user_code($length, $no_duplicate, $insert);
    }
  }

  if (FALSE !== $insert) {
    w_user_code_save($code);
  }

  return $code;
}

/**
 * Save a user-code record to database.
 */
function w_user_code_save($code, $uid = 0, $watchdog = TRUE) {
  if (!w_user_code_is_duplicated($code)) {
    if ($uid && !w_user_code($uid)) {
      $record = new stdClass();
      $record->uid = $uid;
      $record->code = $code;
      drupal_write_record('w_user_code', $record);

      if ($watchdog) {
        watchdog(
          'user-code', 
          t("User code (@code) created successfully."),
          array('@code' => $code),
          WATCHDOG_INFO
        );
      }
      
      return TRUE;
    }

    if ($watchdog) {
      watchdog(
        'user-code', 
        t("Failed to create user code (@code): User already has code."),
        array('@code' => $code),
        WATCHDOG_ERROR
      );
    }
    
    return FALSE;
  }

  if ($watchdog) {
    watchdog(
      'user-code', 
      t("Failed to create user code (@code): It is already existed."),
      array('@code' => $code),
      WATCHDOG_ERROR
    );
  }

  return FALSE;
}
