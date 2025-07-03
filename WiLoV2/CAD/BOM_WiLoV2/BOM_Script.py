import csv
import requests
import time
import json


CLIENT_ID = 'WHzj3hrAxVNbgke1xfhwCHgzwkJ5dc2AtlR3Btkzfi8aGhdI'
CLIENT_SECRET = 'YURV4QOf7dduGWPjaMoExrYtAFvCL3dl3Q5UoejmZAGqzfFnATjLIXzxsBOQH5uK'

# Updated API v4 URLs
TOKEN_URL = 'https://api.digikey.com/v1/oauth2/token'
PRODUCT_DETAILS_URL = 'https://api.digikey.com/products/v4/search/productdetails'  # This endpoint has issues
KEYWORD_SEARCH_URL = 'https://api.digikey.com/products/v4/search/keyword'  # This one works!

def get_access_token():
    headers = {'Content-Type': 'application/x-www-form-urlencoded'}
    data = {
        'client_id': CLIENT_ID,
        'client_secret': CLIENT_SECRET,
        'grant_type': 'client_credentials'
    }
    
    print("Getting access token...")
    response = requests.post(TOKEN_URL, headers=headers, data=data)
    print(f"Token response status: {response.status_code}")
    
    if response.status_code != 200:
        print(f"Token error: {response.text}")
        response.raise_for_status()
    
    token_data = response.json()
    token = token_data.get('access_token')
    if not token:
        raise Exception("Access token not found in response")
    
    print("Access token obtained successfully")
    return token

def get_part_info_v4(mpn, token):
    """Get part info using DigiKey API v4 - KeywordSearch only since ProductDetails endpoint is broken"""
    headers = {
        'Authorization': f'Bearer {token}',
        'X-DIGIKEY-Client-Id': CLIENT_ID,
        'Content-Type': 'application/json',
    }
    
    # Use KeywordSearch directly since it's working and ProductDetails has endpoint issues
    try:
        print(f"Searching for {mpn}")
        json_data = {
            'Keywords': mpn,
            'RecordCount': 1,
            'RecordStartPosition': 0,
            'SearchOptions': [],
            'Sort': {'Option': 'SortByDigiKeyPartNumber', 'Direction': 'Ascending'},
            'RequestedQuantity': 1
        }
        
        response = requests.post(KEYWORD_SEARCH_URL, headers=headers, json=json_data)
        
        if response.status_code == 200:
            data = response.json()
            return parse_keyword_search_v4(data, mpn)
        else:
            print(f"Search failed for {mpn}: {response.status_code} - {response.text[:200]}")
            return None, None, "search_failed"
            
    except Exception as e:
        print(f"Search exception for {mpn}: {e}")
        return None, None, "exception"

def extract_manufacturer_v4(data):
    """Extract manufacturer from v4 ProductDetails response"""
    if 'Manufacturer' in data:
        if isinstance(data['Manufacturer'], dict):
            return data['Manufacturer'].get('Value') or data['Manufacturer'].get('Name')
        return data['Manufacturer']
    return None

def extract_description_v4(data):
    """Extract description from v4 ProductDetails response"""
    if 'ProductDescription' in data:
        return data['ProductDescription']
    elif 'Description' in data:
        return data['Description']
    return None

def parse_keyword_search_v4(data, mpn):
    """Parse v4 KeywordSearch response"""
    
    # Look for products in the response
    products = data.get('Products', [])
    
    if not products:
        print(f"No products found for {mpn}")
        return None, None, "no_results"
    
    # Take the first product
    product = products[0]
    
    # Extract manufacturer - handle nested structure
    manufacturer = None
    if 'Manufacturer' in product and product['Manufacturer']:
        manufacturer = product['Manufacturer'].get('Name')
    
    # Extract description - handle nested structure  
    description = None
    if 'Description' in product and product['Description']:
        description = product['Description'].get('ProductDescription')
        if not description:
            description = product['Description'].get('DetailedDescription')
    
    print(f"Found: {manufacturer} | {description}")
    return manufacturer, description, "keyword_search"

def clean_mpn(mpn):
    """Clean up the MPN"""
    if not mpn:
        return mpn
    return mpn.strip().strip('"').strip("'")

def test_known_part(token):
    """Test with a known DigiKey part number to verify API is working"""
    # Use a common part number that should exist
    test_parts = ["296-8201-1-ND", "311-1.00KCRCT-ND", "C0805C104K5RACTU"]
    
    for part in test_parts:
        print(f"\n=== Testing known part: {part} ===")
        manufacturer, description, method = get_part_info_v4(part, token)
        print(f"Result: {manufacturer} | {description} | {method}")
        if manufacturer or description:
            print("✓ API is working correctly!")
            return True
        time.sleep(1)
    
    print("✗ API test failed - no known parts found")
    return False

def update_bom(input_csv, output_csv):
    token = get_access_token()
    
    # First test the API with known parts
    if not test_known_part(token):
        print("API testing failed. Please check your credentials and API access.")
        return
    
    with open(input_csv, 'r', encoding='latin-1') as infile, open(output_csv, 'w', newline='', encoding='utf-8') as outfile:
        reader = csv.reader(infile, delimiter=';')
        writer = csv.writer(outfile, delimiter=';')
        
        header = next(reader)
        header += ['Manufacturer', 'Description', 'Match_Type']
        writer.writerow(header)
        
        for i, row in enumerate(reader):
            mpn = clean_mpn(row[-1])
            print(f"\nProcessing row {i+1}: {mpn}")
            
            try:
                manufacturer, description, match_type = get_part_info_v4(mpn, token)
            except Exception as e:
                print(f"Error fetching part info for {mpn}: {e}")
                manufacturer, description, match_type = None, None, "exception"
            
            row += [manufacturer or 'N/A', description or 'N/A', match_type or 'N/A']
            writer.writerow(row)
            
            # Be respectful of API rate limits
            time.sleep(2)
            
            # Optional: limit for testing
            # if i >= 5:
            #     break

def debug_single_part(mpn, token):
    """Debug a single part with detailed output"""
    print(f"\n=== DETAILED DEBUG FOR {mpn} ===")
    
    headers = {
        'Authorization': f'Bearer {token}',
        'X-DIGIKEY-Client-Id': CLIENT_ID,
        'Content-Type': 'application/json',
    }
    
    # Test ProductDetails
    print("1. Testing ProductDetails...")
    json_data = {'Part': mpn, 'IncludeAllAssociatedProducts': False}
    response = requests.post(PRODUCT_DETAILS_URL, headers=headers, json=json_data)
    print(f"Status: {response.status_code}")
    print(f"Response: {response.text[:300]}...")
    
    # Test KeywordSearch
    print("\n2. Testing KeywordSearch...")
    json_data = {
        'Keywords': mpn,
        'RecordCount': 1,
        'RecordStartPosition': 0,
        'SearchOptions': [],
        'Sort': {'Option': 'SortByDigiKeyPartNumber', 'Direction': 'Ascending'},
        'RequestedQuantity': 1
    }
    response = requests.post(KEYWORD_SEARCH_URL, headers=headers, json=json_data)
    print(f"Status: {response.status_code}")
    print(f"Response: {response.text[:300]}...")

if __name__ == "__main__":
    try:
        print("Starting BOM update process...")
        update_bom("wiloV4_BOM.csv", "bom_output.csv")
        print("BOM update completed successfully!")
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()